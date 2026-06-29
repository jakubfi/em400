//  Copyright (c) 2026 Jakub Filipowicz <jakubf@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include <QCloseEvent>
#include <QTimer>
#include <QDebug>
#include <QFileDialog>
#include <QSettings>
#include <QAction>
#include <QSignalBlocker>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QShortcut>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "appcfg.h"
#include "emdas.h"
#include "theme.h"
#include "configdialog.h"

// -----------------------------------------------------------------------
namespace {
class SepLine : public QFrame {
public:
	SepLine()
	{
		setFrameShape(QFrame::VLine);
		setFrameShadow(QFrame::Plain);
		apply_color();
	}
protected:
	void changeEvent(QEvent *ev) override
	{
		QFrame::changeEvent(ev);
		if (ev->type() == QEvent::PaletteChange || ev->type() == QEvent::StyleChange) {
			apply_color();
		}
	}
private:
	void apply_color()
	{
		QPalette pal = palette();
		pal.setColor(QPalette::WindowText, em400_sep_color(pal));
		setPalette(pal);
	}
};
}

// -----------------------------------------------------------------------
static QFrame *make_vsep()
{
	return new SepLine();
}

// -----------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	cfg_ctl(&appcfg, &e),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	build_docks();
	init_layout();
	wire_connections();
	build_statusbar();

	// rotary on IC by default
	ui->cp->rotary->set_position(8);

	// do the power-on if started with power on
	if (e.is_powered()) {
		ui->cp->ignition->set_position(1);
		emit ui->cp->ignition->signal_power(true);
	}
	update_window_title();
	update_docks_enabled(e.is_powered());

	restore_layout();
}

// -----------------------------------------------------------------------
void MainWindow::build_docks()
{
	dasm = new DasmView();
	mem = new MemView();
	dasm->connect_emu(&e);
	mem->connect_emu(&e);

	uregs = new RegCompact(&e, RegCompact::USER);
	sregs = new RegCompact(&e, RegCompact::SYSTEM);
	ints = new IntView(&e);
	map = new MapView(&e);
	brk = new BrkView(&e);
	watch = new WatchView(&e);
	stack = new StackView(&e);

	ui->menuView->addSeparator();
	dock_dasm  = register_dock(dasm,  tr("Disassembly"),      "dock_dasm");
	dock_mem   = register_dock(mem,   tr("Memory"),           "dock_mem");
	dock_uregs = register_dock(uregs, tr("User registers"),   "dock_uregs");
	dock_sregs = register_dock(sregs, tr("System registers"), "dock_sregs");
	dock_ints  = register_dock(ints,  tr("Interrupts"),       "dock_ints");
	dock_map   = register_dock(map,   tr("Allocation map"),   "dock_map");
	dock_brk   = register_dock(brk,   tr("Breakpoints"),      "dock_brk");
	dock_watch = register_dock(watch, tr("Watches"),          "dock_watch");
	dock_stack = register_dock(stack, tr("Stack"),            "dock_stack");

	QShortcut *search_sc = new QShortcut(QKeySequence::Find, this);
	connect(search_sc, &QShortcut::activated, this, [this]() {
		dock_mem->show();
		dock_mem->raise();
		mem->open_search();
	});
}

// -----------------------------------------------------------------------
void MainWindow::init_layout()
{
	setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
	setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

	// Allow nested splits so docks can be arranged into sub-columns/rows
	setDockNestingEnabled(true);

	apply_default_layout();

	// Center the fixed-size control panel in whatever central space remains, so it
	// sits in the middle of its column instead of pinned top-left (the panel does
	// not stretch - it blits its art at 0,0 - so without this the extra space shows
	// as blank below/right). The Qt::AlignCenter cell also stops the layout from
	// stretching the panel.
	// The panel must be reparented out of the .ui central widget before
	// setCentralWidget() deletes that old central widget; addWidget() does that.
	QWidget *cp_host = new QWidget(this);
	// Cap the host at the panel's height (Maximum) so the memory dock below claims
	// the rest of the center column; center the panel horizontally but pin it to
	// the top - vertical centering would push memory down and leave a gap.
	cp_host->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	QGridLayout *cp_layout = new QGridLayout(cp_host);
	cp_layout->setContentsMargins(0, 0, 0, 0);
	cp_layout->addWidget(ui->cp, 0, 0, Qt::AlignHCenter | Qt::AlignTop);
	setCentralWidget(cp_host);

	ui->menuView->addSeparator();
	QAction *act_reset = new QAction(tr("Reset Layout"), this);
	ui->menuView->addAction(act_reset);
	connect(act_reset, &QAction::triggered, this, &MainWindow::apply_default_layout);
}

// -----------------------------------------------------------------------
void MainWindow::wire_connections()
{
	// MainWindow -> ControlPanel
	connect(ui->actionLoad_OS_image, &QAction::triggered, this, &MainWindow::load_os_image);
	connect(ui->actionPreferences, &QAction::triggered, this, &MainWindow::open_config);
	connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
	connect(ui->actionSmall_Control_Panel, &QAction::toggled, this, &MainWindow::slot_smallcp_changed);
	connect(ui->actionDebugger, &QAction::toggled, this, &MainWindow::slot_debugger_enabled_changed);
	connect(ui->actionPanel_Theme, &QAction::toggled, this, &MainWindow::slot_panel_theme_changed);

	// EmuModel -> MainWindow
	connect(&e, &EmuModel::signal_reg_changed, this, &MainWindow::slot_cpu_reg_changed);
	// The dasm "skip" bar reads P live at paint time, so it only needs to repaint
	// when IC moves (slot_cpu_reg_changed gates on IC). Do NOT also repaint on
	// signal_p_changed: P is sampled by the faster realtime timer, so it would
	// repaint with a stale (not-yet-advanced) IC and flash the bar at the wrong row.
	connect(&e, &EmuModel::signal_cpu_ips_tick, this, &MainWindow::slot_ips_update);
	connect(&e, &EmuModel::signal_mc_changed, this, &MainWindow::update_mc_status);

	// EmuModel -> ControlPanel
	connect(&e, &EmuModel::signal_state_changed, ui->cp, &ControlPanel::slot_state_changed);
	connect(&e, &EmuModel::signal_bus_w_changed, ui->cp->wleds, &BusWLeds::slot_set_value);
	connect(&e, &EmuModel::signal_alarm_changed, ui->cp, &ControlPanel::slot_set_alarm);
	connect(&e, &EmuModel::signal_p_changed, ui->cp, &ControlPanel::slot_set_p);
	connect(&e, &EmuModel::signal_clock_changed, ui->cp, &ControlPanel::slot_set_clock);

	// ControlPanel -> EmuModel
	// the ignition's power signal drives the machine lifecycle (the lock signal
	// is panel-internal, handled in ControlPanel itself)
	connect(ui->cp->ignition, &Ignition::signal_power, &e, &EmuModel::slot_power);
	// power-on resets the core's selected display register, so re-push the rotary
	// position; connected after slot_power so it runs once the machine is powered.
	connect(ui->cp->ignition, &Ignition::signal_power, this, [this](bool on) {
		if (on) e.slot_reg_select(ui->cp->rotary->get_position());
	});
	connect(&e, &EmuModel::signal_power_changed, this, &MainWindow::update_window_title);
	connect(&e, &EmuModel::signal_power_changed, this, &MainWindow::update_docks_enabled);
	connect(ui->cp, &ControlPanel::signal_start_toggled, &e, &EmuModel::slot_cpu_start);
	connect(ui->cp, &ControlPanel::signal_clear_clicked, &e, &EmuModel::slot_clear);
	connect(ui->cp, &ControlPanel::signal_oprq_clicked,  &e, &EmuModel::slot_oprq);
	connect(ui->cp, &ControlPanel::signal_cycle_clicked, &e, &EmuModel::slot_cycle);
	connect(ui->cp, &ControlPanel::signal_load_clicked, &e, &EmuModel::slot_load);
	connect(ui->cp, &ControlPanel::signal_fetch_clicked, &e, &EmuModel::slot_fetch);
	connect(ui->cp, &ControlPanel::signal_store_clicked, &e, &EmuModel::slot_store);
	connect(ui->cp, &ControlPanel::signal_bin_clicked, &e, &EmuModel::slot_bin);
	connect(ui->cp, &ControlPanel::signal_clock_toggled, &e, &EmuModel::slot_clock_enabled);
	connect(ui->cp->rotary, &Rotary::signal_rotated, &e, &EmuModel::slot_reg_select);
	connect(ui->cp->keys, &BinaryKeys::signal_value_changed, &e, &EmuModel::set_kb);

	// register edits are handled inside the RegView models (setData -> set_reg)

	// clicking a page in the allocation map jumps the memory view to its start
	connect(map, &MapView::signal_page_clicked, mem, &MemView::update_contents);

	// "Locate in Memory View" from the disassembly context menu: jump the memory
	// view there, highlight the cell, and surface the (possibly hidden) dock
	connect(dasm, &DasmView::signal_locate_in_memory, this, [this](int nb, int addr) {
		mem->locate_cell(nb, addr);
		dock_mem->show();
		dock_mem->raise();
	});
}

// -----------------------------------------------------------------------
void MainWindow::build_statusbar()
{
	QFont font("Monospace");

	ips = new QLabel();
	ui->statusbar->addWidget(ips);
	ui->statusbar->addWidget(make_vsep());

	QLabel *flags_label = new QLabel(QString("<span>%1</span>").arg(tr("Flags:")));
	flags_label->setFont(font);
	ui->statusbar->addWidget(flags_label);
	flags = new QLabel();
	flags->setFont(font);
	ui->statusbar->addWidget(flags);
	ui->statusbar->addWidget(make_vsep());

	QLabel *status_label = new QLabel(QString("<span>%1</span>").arg(tr("Status:")));
	status_label->setFont(font);
	ui->statusbar->addWidget(status_label);
	q = new QLabel("<span>Q</span>");
	q->setFont(font);
	q->setToolTip(tr("User-mode flag"));
	ui->statusbar->addWidget(q);
	bs = new QLabel("<span>BS</span>");
	bs->setFont(font);
	ui->statusbar->addWidget(bs);
	nb = new QLabel("<span>NB=0</span>");
	nb->setFont(font);
	nb->setToolTip(tr("Current memory segment"));
	ui->statusbar->addWidget(nb);
	ui->statusbar->addWidget(make_vsep());

	mc = new QLabel("<span>MC=0</span>");
	mc->setFont(font);
	mc->setToolTip(tr("Modification Counter"));
	ui->statusbar->addWidget(mc);
	ui->statusbar->addWidget(make_vsep());

	// Memory editor mode indicator
	edit_box = new QWidget();
	QHBoxLayout *edit_lay = new QHBoxLayout(edit_box);
	edit_lay->setContentsMargins(0, 0, 0, 0);
	edit_lay->setSpacing(6);
	edit_lay->addWidget(make_vsep());
	edit_mode = new QLabel();
	edit_lay->addWidget(edit_mode);
	ui->statusbar->addPermanentWidget(edit_box);
	edit_box->setVisible(false);
	connect(mem, &MemView::signal_edit_mode_changed, this, &MainWindow::slot_edit_mode_changed);
}

// -----------------------------------------------------------------------
// Restore the user's last layout. On the very first run there is no saved
// state, so we start with the control panel only (debugger off) - that is the
// default. After that the user's arrangement is restored verbatim: dock
// positions, sizes, visibility and float state all come back as left.
void MainWindow::restore_layout()
{
	QSettings settings;
	// reflect the panel-theme choice in the menu without re-applying it: the
	// theme was already set on the QApplication in ui_qt6_loop() before this
	// window was constructed, so just sync the checkbox state.
	{
		QSignalBlocker block(ui->actionPanel_Theme);
		ui->actionPanel_Theme->setChecked(settings.value("ui/panelTheme", true).toBool());
	}
	ui->cp->set_volume(settings.value("ui/guiVolume", 100).toInt());
	// restore the small/large panel choice first; setChecked() fires the toggled
	// signal which applies the crop (only when it actually differs from default)
	ui->actionSmall_Control_Panel->setChecked(settings.value("layout/smallPanel", false).toBool());
	if (settings.contains("layout/windowState")) {
		// Documented order: geometry before state.
		QByteArray geo = settings.value("layout/geometry").toByteArray();
		restoreGeometry(geo);
		restoreState(settings.value("layout/windowState").toByteArray());
		sync_debugger_action();
		// Re-settle the size once the event loop has run (inline races the dock
		// layout). With the debugger visible, re-apply the saved geometry verbatim:
		// adjustSize() would hug the content height, and memory's height hint is
		// tiny, so the window would collapse to a sliver showing only its top row.
		// Honoring the saved size keeps memory tall (and preserves hand-sized
		// docks). With only the panel shown the size is derived from the panel, so
		// shrink-wrap to it.
		bool dbg = ui->actionDebugger->isChecked();
		QTimer::singleShot(0, this, [this, dbg, geo]() {
			if (isMaximized() || isFullScreen()) return;
			if (dbg) restoreGeometry(geo);
			else adjustSize();
		});
	} else {
		slot_debugger_enabled_changed(false);
		sync_debugger_action();
	}
}

// -----------------------------------------------------------------------
MainWindow::~MainWindow()
{
	delete ui;
}

// -----------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent* event)
{
	// persist whatever arrangement the user settled on
	QSettings settings;
	settings.setValue("layout/geometry", saveGeometry());
	settings.setValue("layout/windowState", saveState());
	settings.setValue("layout/smallPanel", ui->actionSmall_Control_Panel->isChecked());

	e.stop();
	event->accept();
}

// -----------------------------------------------------------------------
// Wrap a debugger widget in a QDockWidget
QDockWidget *MainWindow::register_dock(QWidget *view, const QString &title, const QString &objname)
{
	QDockWidget *dock = new QDockWidget(title, this);
	dock->setObjectName(objname);
	dock->setWidget(view);
	ui->menuView->addAction(dock->toggleViewAction());
	docks.append(dock);
	return dock;
}

// -----------------------------------------------------------------------
void MainWindow::apply_default_layout()
{
	addDockWidget(Qt::LeftDockWidgetArea, dock_dasm); // narrow, full height

	addDockWidget(Qt::BottomDockWidgetArea, dock_brk); // under panel, left of memory
	splitDockWidget(dock_brk, dock_mem, Qt::Horizontal); // memory beside breakpoints
	splitDockWidget(dock_brk, dock_watch, Qt::Vertical); // watches under breakpoints

	addDockWidget(Qt::RightDockWidgetArea, dock_uregs); // small modules, stacked
	addDockWidget(Qt::RightDockWidgetArea, dock_sregs);
	addDockWidget(Qt::RightDockWidgetArea, dock_ints);
	addDockWidget(Qt::RightDockWidgetArea, dock_map);
	// tab the stack behind the interrupts view
	// to keep the right column from running too tall.
	tabifyDockWidget(dock_ints, dock_stack);

	for (QDockWidget *d : docks) {
		d->setFloating(false);
		d->show();
	}

	// brk/watch widths hug their content (their own size policies cap horizontal growth)
	// and memory expands to take the rest, so the column is sized independently of memory.

	// show interrupts as the front tab of the interrupts/stack pair
	dock_ints->raise();

	sync_debugger_action();
}

// -----------------------------------------------------------------------
// Keep the master "Debugger" menu check in sync with the docks without letting
// it re-fire slot_debugger_enabled_changed (which would clobber the layout).
void MainWindow::sync_debugger_action()
{
	bool any = false;
	for (QDockWidget *d : docks) {
		if (!d->isHidden()) {
			any = true;
			break;
		}
	}
	QSignalBlocker block(ui->actionDebugger);
	ui->actionDebugger->setChecked(any);
}

// -----------------------------------------------------------------------
void MainWindow::update_mc_status(int value)
{
	mc->setText(QString("<span>MC=%1</span>").arg(value));
}

// -----------------------------------------------------------------------
void MainWindow::update_sr_status(uint16_t sr)
{
	int vnb = sr & 0b1111;
	int vbs = sr & 0b10000;
	int vq = sr & 0b100000;

	if (vq) q->setStyleSheet("font-weight: bold; color: palette(text);");
	else q->setStyleSheet("font-weight: normal; color: palette(mid);");

	if (vbs) bs->setStyleSheet("font-weight: bold; color: palette(text);");
	else bs->setStyleSheet("font-weight: normal; color: palette(mid);");

	nb->setText(QString("<span>NB=%1</span>").arg(vnb));
}

// -----------------------------------------------------------------------
void MainWindow::update_r0_status(uint16_t r0)
{
	QString f = "ZMVCLEGYX1234567";
	QString txt;
	for (int i=0 ; i<16 ; i++) {
		if (r0 & 1<<(15-i))	txt.append(QString("<b>%1</b>").arg(f[i]));
		else txt.append(QString("<font color=gray>%1</font>").arg(f[i]));
	}
	flags->setText(txt);
}

// -----------------------------------------------------------------------
void MainWindow::slot_cpu_reg_changed(int reg, uint16_t val)
{
	// The RegViews render the values themselves; here we only drive the
	// derived status-bar decode (R0 flags, SR) and refresh the disassembly.
	// Only IC moves the dasm bar - repainting on every register (which fire
	// before IC in the sync loop) would briefly paint the bar at the stale IC.
	if (reg == EM400_REG_IC) {
		slot_dasm_update();
	}

	if (reg == EM400_REG_R0) {
		update_r0_status(val);
	} else if (reg == EM400_REG_SR) {
		update_sr_status(val);
	}
}

// -----------------------------------------------------------------------
void MainWindow::slot_dasm_update()
{
	int qnb = e.get_qnb();
	int ic = e.get_reg(EM400_REG_IC);
	dasm->update_contents(qnb, ic);
}

// -----------------------------------------------------------------------
void MainWindow::slot_ips_update(unsigned long ips)
{
	QString mips_t = QString("%1 MIPS").arg(QString::number(ips/1000000.0, 'f', 3));
	this->ips->setText(mips_t);
}

// -----------------------------------------------------------------------
void MainWindow::slot_edit_mode_changed(bool editing, bool insert)
{
	edit_box->setVisible(editing);
	edit_mode->setText(insert ? tr("Edit: insert") : tr("Edit: overwrite"));
}

// -----------------------------------------------------------------------
void MainWindow::update_window_title()
{
	struct appcfg_machine *m = appcfg_machine_find(&appcfg, appcfg.active_id);
	const char *label = m ? (m->name ? m->name : m->id) : nullptr;

	QString title = QStringLiteral("EM400");
	if (label && *label) {
		title += QString(" — %1").arg(QString::fromUtf8(label));
	}
	if (!e.is_powered()) {
		title += tr(" (powered off)");
	}
	setWindowTitle(title);
}

// -----------------------------------------------------------------------
// Powered-off machine has no state to inspect or edit, so make the debugger
// dock contents inert (and visibly greyed). The title bars stay live so docks
// can still be moved, floated or closed.
void MainWindow::update_docks_enabled(bool powered)
{
	for (QDockWidget *d : docks) {
		if (d->widget()) d->widget()->setEnabled(powered);
	}
}

// -----------------------------------------------------------------------
void MainWindow::open_config()
{
	if (config_dialog) {
		config_dialog->raise();
		config_dialog->activateWindow();
		return;
	}

	config_dialog = new ConfigDialog(&cfg_ctl, this);
	connect(config_dialog, &QDialog::finished, config_dialog, &QObject::deleteLater);
	connect(&e, &EmuModel::signal_power_changed, config_dialog, &ConfigDialog::update_enabled_states);
	connect(config_dialog, &ConfigDialog::signal_machine_renamed, this, &MainWindow::update_window_title);
	connect(config_dialog, &ConfigDialog::signal_gui_volume_changed, ui->cp, &ControlPanel::set_volume);
	config_dialog->show();
	config_dialog->raise();
	config_dialog->activateWindow();
}

// -----------------------------------------------------------------------
void MainWindow::load_os_image()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Open OS image..."), nullptr, nullptr);
	if (filename.isNull()) {
		return;
	}

	QFileInfo fi(filename);

	e.load_os_image(filename);
}

// -----------------------------------------------------------------------
void MainWindow::slot_debugger_enabled_changed(bool state)
{
	for (QDockWidget *d : docks) {
		d->setVisible(state);
	}
	//ui->statusbar->setVisible(state);
	for (int i=0 ; i<10 ; i++) qApp->processEvents(); // StackOverflow, I don't even...
	adjustSize();
}

// -----------------------------------------------------------------------
void MainWindow::slot_smallcp_changed(bool state)
{
	ui->cp->slot_small_panel_changed(state);
	int h = height();
	for (int i=0 ; i<10 ; i++) qApp->processEvents(); // StackOverflow, I don't even...
	adjustSize();
	// adjustSize re-fits the width to the resized panel, but it also hugs the
	// content height - which collapses the window onto memory's tiny height hint.
	// Keep the debugger height; panel-only has no such content so let it shrink.
	if (ui->actionDebugger->isChecked()) resize(width(), h);
}

// -----------------------------------------------------------------------
void MainWindow::slot_panel_theme_changed(bool state)
{
	em400_apply_theme(state);
	QSettings settings;
	settings.setValue("ui/panelTheme", state);
}

// vim: tabstop=4 shiftwidth=4 autoindent

#include <QCloseEvent>
#include <QTimer>
#include <QDebug>
#include <QFileDialog>
#include <QSettings>
#include <QAction>
#include <QSignalBlocker>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QFrame>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "emdas.h"
#include "switch.h"
#include "theme.h"

// -----------------------------------------------------------------------
// A flat vertical separator that tracks the theme. A plain (non-bevelled)
// VLine draws using QPalette::WindowText, which we set to em400_sep_color()
// (muted red in the panel theme, palette Mid grey under the system theme).
// Because that color differs between themes, we re-pull it on every palette
// change so a live View > Panel Theme toggle updates the separators too.
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

static QFrame *make_vsep()
{
	return new SepLine();
}

// -----------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	ui->dasm->connect_emu(&e);
	ui->mem->connect_emu(&e);

	uregs = new RegCompact(&e, RegCompact::USER);
	sregs = new RegCompact(&e, RegCompact::SYSTEM);
	ints = new IntView(&e);
	map = new MapView(&e);
	brk = new BrkView(&e);
	watch = new WatchView(&e);

	// Re-home the debugger panels into dock widgets arranged around the
	// control panel. The control panel is the permanent center - it IS the
	// machine - while the debugger modules dock around it and can be moved,
	// floated, tabbed or hidden. Reparenting the existing group boxes via
	// setWidget() keeps all their child pointers (ui->dasm, ui->mem) and
	// signal wiring intact; only their host changes.
	dock_uregs = new QDockWidget(tr("User registers"), this);
	dock_sregs = new QDockWidget(tr("System registers"), this);
	dock_dasm = new QDockWidget(tr("Disassembly"), this);
	dock_mem  = new QDockWidget(tr("Memory"), this);
	dock_ints = new QDockWidget(tr("Interrupts"), this);
	dock_map  = new QDockWidget(tr("Allocation map"), this);
	dock_brk  = new QDockWidget(tr("Breakpoints"), this);
	dock_watch = new QDockWidget(tr("Watches"), this);

	dock_uregs->setObjectName("dock_uregs");
	dock_sregs->setObjectName("dock_sregs");
	dock_dasm->setObjectName("dock_dasm");
	dock_mem->setObjectName("dock_mem");
	dock_ints->setObjectName("dock_ints");
	dock_map->setObjectName("dock_map");
	dock_brk->setObjectName("dock_brk");
	dock_watch->setObjectName("dock_watch");

	// dock title bars now carry the names; drop the redundant group titles
	ui->group_dasm->setTitle("");
	ui->group_mem->setTitle("");

	dock_uregs->setWidget(uregs);
	dock_sregs->setWidget(sregs);
	dock_ints->setWidget(ints);
	dock_map->setWidget(map);
	dock_brk->setWidget(brk);
	dock_watch->setWidget(watch);
	dock_dasm->setWidget(ui->group_dasm);
	dock_mem->setWidget(ui->group_mem);

	// Space preferences drive the default arrangement:
	//   disassembly is the priority - narrow but wants all the vertical it can
	//     get, so it owns the full-height left column.
	//   memory is lower priority and likes to be wide and tall, so it docks below
	//     the fixed panel in the center column (inherits the panel width, takes the
	//     height left under the panel).
	//   registers (and future watches/breakpoints) are small -> full-height right
	//     column, stacked.
	// Hand each side area its corners so the left (dasm) and right (regs) columns
	// run the full window height; that confines the bottom area to the center
	// column, which is exactly where memory belongs (under the panel).
	setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
	setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

	// Allow nested splits so docks can be arranged into sub-columns/rows inside a
	// single area - e.g. breakpoints stacked directly under watches beside the
	// memory view. Without this a dock can only stack along an edge area, not
	// split a region that is itself a split.
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

	// per-module show/hide entries in the View menu (re-open a closed dock),
	// plus an escape hatch back to the curated arrangement for anyone who drags
	// the docks into a corner and can't find their way out.
	ui->menuView->addSeparator();
	ui->menuView->addAction(dock_dasm->toggleViewAction());
	ui->menuView->addAction(dock_mem->toggleViewAction());
	ui->menuView->addAction(dock_uregs->toggleViewAction());
	ui->menuView->addAction(dock_sregs->toggleViewAction());
	ui->menuView->addAction(dock_ints->toggleViewAction());
	ui->menuView->addAction(dock_map->toggleViewAction());
	ui->menuView->addAction(dock_brk->toggleViewAction());
	ui->menuView->addAction(dock_watch->toggleViewAction());
	ui->menuView->addSeparator();
	QAction *act_reset = new QAction(tr("Reset Layout"), this);
	ui->menuView->addAction(act_reset);
	connect(act_reset, &QAction::triggered, this, &MainWindow::apply_default_layout);

	// MainWindow -> ControlPanel
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
	connect(&e, &EmuModel::signal_bus_w_changed, ui->cp, &ControlPanel::slot_bus_w_changed);
	connect(&e, &EmuModel::signal_alarm_changed, ui->cp->led[LED_ALARM], &LED::slot_change);
	connect(&e, &EmuModel::signal_p_changed, ui->cp->led[LED_P], &LED::slot_change);
	connect(&e, &EmuModel::signal_clock_changed, ui->cp->led[LED_CLOCK], &LED::slot_change);

	// ControlPanel -> EmuModel
	connect(ui->cp->sw[SW_START], &Switch::signal_toggled, &e, &EmuModel::slot_cpu_start);
	connect(ui->cp->sw[SW_CLEAR], &Switch::signal_clicked, &e, &EmuModel::slot_clear);
	connect(ui->cp->sw[SW_OPRQ],  &Switch::signal_clicked, &e, &EmuModel::slot_oprq);
	connect(ui->cp->sw[SW_CYCLE], &Switch::signal_clicked, &e, &EmuModel::slot_cycle);
	connect(ui->cp->sw[SW_LOAD], &Switch::signal_clicked, &e, &EmuModel::slot_load);
	connect(ui->cp->sw[SW_FETCH], &Switch::signal_clicked, &e, &EmuModel::slot_fetch);
	connect(ui->cp->sw[SW_STORE], &Switch::signal_clicked, &e, &EmuModel::slot_store);
	connect(ui->cp->sw[SW_BIN], &Switch::signal_clicked, &e, &EmuModel::slot_bin);
	connect(ui->cp->sw[SW_CLOCK], &Switch::signal_toggled, &e, &EmuModel::slot_clock_enabled);
	connect(ui->cp->rotary, &Rotary::signal_rotated, &e, &EmuModel::slot_reg_select);
	for (int i=0 ; i<16 ; i++) {
		connect(ui->cp->sw[i], &Switch::signal_toggled, this, &MainWindow::slot_binary_key_toggled);
	}

	// register edits are handled inside the RegView models (setData -> set_reg)

	// status bar: emulation state on the left (MIPS, flags, status, MC),
	// grouped with vertical separators; non-emulation indicators (memory
	// editor mode) pushed to the right.
	QFont font("Monospace");

	ips = new QLabel();
	ui->statusbar->addWidget(ips);
	ui->statusbar->addWidget(make_vsep());

	QLabel *flags_label = new QLabel("<span>Flags:</span>");
	flags_label->setFont(font);
	ui->statusbar->addWidget(flags_label);
	flags = new QLabel();
	flags->setFont(font);
	ui->statusbar->addWidget(flags);
	ui->statusbar->addWidget(make_vsep());

	QLabel *status_label = new QLabel("<span>Status:</span>");
	status_label->setFont(font);
	ui->statusbar->addWidget(status_label);
	q = new QLabel("<span>Q</span>");
	q->setFont(font);
	ui->statusbar->addWidget(q);
	bs = new QLabel("<span>BS</span>");
	bs->setFont(font);
	ui->statusbar->addWidget(bs);
	nb = new QLabel("<span>NB=0</span>");
	nb->setFont(font);
	ui->statusbar->addWidget(nb);
	ui->statusbar->addWidget(make_vsep());

	mc = new QLabel("<span>MC=0</span>");
	mc->setFont(font);
	ui->statusbar->addWidget(mc);
	ui->statusbar->addWidget(make_vsep());

	// Memory editor mode indicator, right-aligned in the default proportional
	// font. Its leading separator and label are hidden together when no cell
	// is being edited.
	edit_box = new QWidget();
	QHBoxLayout *edit_lay = new QHBoxLayout(edit_box);
	edit_lay->setContentsMargins(0, 0, 0, 0);
	edit_lay->setSpacing(6);
	edit_lay->addWidget(make_vsep());
	edit_mode = new QLabel();
	edit_lay->addWidget(edit_mode);
	ui->statusbar->addPermanentWidget(edit_box);
	edit_box->setVisible(false);
	connect(ui->mem, &MemView::signal_edit_mode_changed, this, &MainWindow::slot_edit_mode_changed);

	e.run();
	ui->cp->rotary->set_position(8);
	ui->cp->sw[SW_CLOCK]->set(e.get_clock());

	// Restore the user's last layout. On the very first run there is no saved
	// state, so we start with the control panel only (debugger off) - that is the
	// default. After that the user's arrangement is restored verbatim: dock
	// positions, sizes, visibility and float state all come back as left.
	QSettings settings;
	// reflect the panel-theme choice in the menu without re-applying it: the
	// theme was already set on the QApplication in ui_qt6_loop() before this
	// window was constructed, so just sync the checkbox state.
	{
		QSignalBlocker block(ui->actionPanel_Theme);
		ui->actionPanel_Theme->setChecked(settings.value("ui/panelTheme", true).toBool());
	}
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
// Place the debugger docks in the curated default arrangement and show them.
// Used at startup to establish a home for each dock, and by View > Reset Layout.
// Left column: disassembly (full height) with a short breakpoint list under it
// (semantically tied to the code, and usually only 1-3 entries). Center bottom:
// memory with the watch list beside it. Right column: the register / interrupt
// / map modules, stacked. Users can still drag any of this around (nested docks
// are enabled), this is just the starting point.
void MainWindow::apply_default_layout()
{
	addDockWidget(Qt::LeftDockWidgetArea, dock_dasm);      // narrow, full height
	splitDockWidget(dock_dasm, dock_brk, Qt::Vertical);    // breakpoints under dasm

	addDockWidget(Qt::BottomDockWidgetArea, dock_mem);     // panel width, under panel
	splitDockWidget(dock_mem, dock_watch, Qt::Horizontal); // watches beside memory

	addDockWidget(Qt::RightDockWidgetArea, dock_uregs);    // small modules, stacked
	addDockWidget(Qt::RightDockWidgetArea, dock_sregs);
	addDockWidget(Qt::RightDockWidgetArea, dock_ints);
	addDockWidget(Qt::RightDockWidgetArea, dock_map);
	// registers / interrupts / map stack vertically down the right column (no
	// tabbing); they all fit one below the other given a little window height

	for (QDockWidget *d : {dock_dasm, dock_mem, dock_uregs, dock_sregs, dock_ints, dock_map, dock_brk, dock_watch}) {
		d->setFloating(false);
		d->show();
	}

	// breakpoints sit under dasm but the list is usually 1-3 rows: shrink it to
	// as little height as it will take so dasm keeps the rest of the left column
	resizeDocks({dock_brk}, {1}, Qt::Vertical);
	// memory is the focus of the bottom row; give watches a moderate slice beside it
	resizeDocks({dock_mem, dock_watch}, {2, 1}, Qt::Horizontal);

	sync_debugger_action();
}

// -----------------------------------------------------------------------
// Keep the master "Debugger" menu check in sync with the docks without letting
// it re-fire slot_debugger_enabled_changed (which would clobber the layout).
void MainWindow::sync_debugger_action()
{
	bool any = !dock_dasm->isHidden() || !dock_mem->isHidden()
		|| !dock_uregs->isHidden() || !dock_sregs->isHidden()
		|| !dock_ints->isHidden() || !dock_map->isHidden()
		|| !dock_brk->isHidden() || !dock_watch->isHidden();
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
	ui->dasm->update_contents(qnb, ic);
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
	edit_mode->setText(insert ? "Edit: insert" : "Edit: overwrite");
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
	dock_uregs->setVisible(state);
	dock_sregs->setVisible(state);
	dock_dasm->setVisible(state);
	dock_mem->setVisible(state);
	dock_ints->setVisible(state);
	dock_map->setVisible(state);
	dock_brk->setVisible(state);
	dock_watch->setVisible(state);
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

// -----------------------------------------------------------------------
void MainWindow::slot_binary_key_toggled(bool state)
{
	uint16_t val = 0;
	for (int i=0 ; i<16 ; i++) {
		val |= ui->cp->sw[i]->get() << (15-i);
	}
	e.set_kb(val);
}

// vim: tabstop=4 shiftwidth=4 autoindent

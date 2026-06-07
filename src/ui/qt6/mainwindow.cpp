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

	build_docks();
	init_layout();
	wire_connections();
	build_statusbar();

	e.run();
	ui->cp->rotary->set_position(8);

	restore_layout();
}

// -----------------------------------------------------------------------
// Build every debugger view and home it into a dock widget arranged around the
// control panel. The control panel is the permanent center - it IS the machine -
// while the debugger modules dock around it and can be moved, floated, tabbed or
// hidden. register_dock() builds the dock around a view, gives it a View-menu
// show/hide entry, and collects it into `docks` - so a dock and its menu entry
// are born together, and every place that walks all docks (default layout,
// debugger show/hide, sync) just iterates `docks`. Adding a new module is then:
// build the view, register_dock() it. The order of these calls is the order of
// the entries in the View menu. The named handles are kept only for the bespoke
// default arrangement in apply_default_layout().
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
}

// -----------------------------------------------------------------------
// Establish the window's dock geometry, lay the docks out in their default
// arrangement, host the control panel in the center, and add the Reset Layout
// escape hatch.
void MainWindow::init_layout()
{
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

	// An escape hatch back to the curated arrangement, for anyone who drags the
	// docks into a corner and can't find their way out. (The per-dock show/hide
	// entries above it were added by register_dock().)
	ui->menuView->addSeparator();
	QAction *act_reset = new QAction(tr("Reset Layout"), this);
	ui->menuView->addAction(act_reset);
	connect(act_reset, &QAction::triggered, this, &MainWindow::apply_default_layout);
}

// -----------------------------------------------------------------------
// Wire the signal/slot graph between the menu actions, the EmuModel and the
// control panel.
void MainWindow::wire_connections()
{
	// MainWindow -> ControlPanel
	connect(ui->actionLoad_OS_image, &QAction::triggered, this, &MainWindow::load_os_image);
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
// Build the status bar: emulation state on the left (MIPS, flags, status, MC),
// grouped with vertical separators; non-emulation indicators (memory editor
// mode) pushed to the right.
void MainWindow::build_statusbar()
{
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
// Wrap a debugger view in a QDockWidget: set the title and object name (the
// latter is what saveState()/restoreState() key the layout on), hand the view
// to the dock, add a show/hide entry for it to the View menu, and remember the
// dock in `docks` so the rest of MainWindow can treat all docks uniformly.
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
	// tab the stack behind the interrupts view (both are occasional, event-driven
	// views) to keep the right column from running too tall.
	tabifyDockWidget(dock_ints, dock_stack);
	// registers / interrupts / map stack vertically down the right column (no
	// tabbing); they all fit one below the other given a little window height

	for (QDockWidget *d : docks) {
		d->setFloating(false);
		d->show();
	}

	// breakpoints sit under dasm but the list is usually 1-3 rows: shrink it to
	// as little height as it will take so dasm keeps the rest of the left column
	resizeDocks({dock_brk}, {1}, Qt::Vertical);
	// memory is the focus of the bottom row; give watches a moderate slice beside it
	resizeDocks({dock_mem, dock_watch}, {2, 1}, Qt::Horizontal);

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

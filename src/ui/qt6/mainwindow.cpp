#include <QCloseEvent>
#include <QTimer>
#include <QDebug>
#include <QFileDialog>
#include <QSettings>
#include <QAction>
#include <QSignalBlocker>
#include <QGridLayout>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "emdas.h"
#include "switch.h"

// -----------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	ui->dasm->connect_emu(&e);
	ui->mem->connect_emu(&e);

	// Dense register tables: user registers (R0-R7, all four bases) and system
	// registers (IC/AR/AC/IR/KB/SR/RZ, sparse). These replace the old
	// one-spinbox-per-register layout; the .ui group_registers box is no longer
	// used (it is dropped with the old central widget on setCentralWidget).
	uregs = new RegCompact(&e, RegCompact::USER);
	sregs = new RegCompact(&e, RegCompact::SYSTEM);

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

	dock_uregs->setObjectName("dock_uregs");
	dock_sregs->setObjectName("dock_sregs");
	dock_dasm->setObjectName("dock_dasm");
	dock_mem->setObjectName("dock_mem");

	// dock title bars now carry the names; drop the redundant group titles
	ui->group_dasm->setTitle("");
	ui->group_mem->setTitle("");

	dock_uregs->setWidget(uregs);
	dock_sregs->setWidget(sregs);
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
	ui->menuView->addSeparator();
	QAction *act_reset = new QAction(tr("Reset Layout"), this);
	ui->menuView->addAction(act_reset);
	connect(act_reset, &QAction::triggered, this, &MainWindow::apply_default_layout);

	// MainWindow -> ControlPanel
	connect(ui->actionSmall_Control_Panel, &QAction::toggled, this, &MainWindow::slot_smallcp_changed);
	connect(ui->actionDebugger, &QAction::toggled, this, &MainWindow::slot_debugger_enabled_changed);

	// EmuModel -> MainWindow
	connect(&e, &EmuModel::signal_reg_changed, this, &MainWindow::slot_cpu_reg_changed);
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

	// status bar contents
	ips = new QLabel();
	ui->statusbar->addPermanentWidget(ips);

	QFont font("Monospace");

	QLabel *flags_label = new QLabel("<span>Flags:</span>");
	flags_label->setFont(font);
	ui->statusbar->addWidget(flags_label);
	flags = new QLabel();
	flags->setFont(font);
	ui->statusbar->addWidget(flags);

	QLabel *status_label = new QLabel(" <span>Status:</span>");
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
	// MC (modification counter) is internal and not editable, so it lives here
	// rather than in the register tables. Only the MC LED (on/off) is exposed
	// by libem400, not the 0-3 count.
	mc = new QLabel("<span>MC</span>");
	mc->setFont(font);
	ui->statusbar->addWidget(mc);

	e.run();
	ui->cp->rotary->set_position(8);
	ui->cp->sw[SW_CLOCK]->set(e.get_clock());

	// Restore the user's last layout. On the very first run there is no saved
	// state, so we start with the control panel only (debugger off) - that is the
	// default. After that the user's arrangement is restored verbatim: dock
	// positions, sizes, visibility and float state all come back as left.
	QSettings settings;
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
// Place the debugger docks in the curated default arrangement (dasm left,
// memory under the panel, registers right) and show them. Used at startup to
// establish a home for each dock, and by the View > Reset Layout action.
void MainWindow::apply_default_layout()
{
	addDockWidget(Qt::LeftDockWidgetArea, dock_dasm);   // narrow, full height
	addDockWidget(Qt::BottomDockWidgetArea, dock_mem);  // panel width, under panel
	addDockWidget(Qt::RightDockWidgetArea, dock_uregs); // small modules, stacked
	addDockWidget(Qt::RightDockWidgetArea, dock_sregs);

	for (QDockWidget *d : {dock_dasm, dock_mem, dock_uregs, dock_sregs}) {
		d->setFloating(false);
		d->show();
	}
	sync_debugger_action();
}

// -----------------------------------------------------------------------
// Keep the master "Debugger" menu check in sync with the docks without letting
// it re-fire slot_debugger_enabled_changed (which would clobber the layout).
void MainWindow::sync_debugger_action()
{
	bool any = !dock_dasm->isHidden() || !dock_mem->isHidden()
		|| !dock_uregs->isHidden() || !dock_sregs->isHidden();
	QSignalBlocker block(ui->actionDebugger);
	ui->actionDebugger->setChecked(any);
}

// -----------------------------------------------------------------------
void MainWindow::update_mc_status(bool on)
{
	if (on) mc->setStyleSheet("font-weight: bold; color: black;");
	else mc->setStyleSheet("font-weight: normal; color: gray;");
}

// -----------------------------------------------------------------------
void MainWindow::update_sr_status(uint16_t sr)
{
	int vnb = sr & 0b1111;
	int vbs = sr & 0b10000;
	int vq = sr & 0b100000;

	if (vq) q->setStyleSheet("font-weight: bold; color: black;");
	else q->setStyleSheet("font-weight: normal; color: gray;");

	if (vbs) bs->setStyleSheet("font-weight: bold; color: black;");
	else bs->setStyleSheet("font-weight: normal; color: gray;");

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
	slot_dasm_update();

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
void MainWindow::slot_binary_key_toggled(bool state)
{
	uint16_t val = 0;
	for (int i=0 ; i<16 ; i++) {
		val |= ui->cp->sw[i]->get() << (15-i);
	}
	e.set_kb(val);
}

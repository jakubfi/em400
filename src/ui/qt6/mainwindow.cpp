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

	r[EM400_REG_R0] = ui->r0;
	r[EM400_REG_R1] = ui->r1;
	r[EM400_REG_R2] = ui->r2;
	r[EM400_REG_R3] = ui->r3;
	r[EM400_REG_R4] = ui->r4;
	r[EM400_REG_R5] = ui->r5;
	r[EM400_REG_R6] = ui->r6;
	r[EM400_REG_R7] = ui->r7;

	r[EM400_REG_IC] = ui->ic;
	r[EM400_REG_AC] = ui->ac;
	r[EM400_REG_AR] = ui->ar;
	r[EM400_REG_IR] = ui->ir;
	r[EM400_REG_SR] = ui->sr;
	r[EM400_REG_RZ] = ui->rz;

	ui->dasm->connect_emu(&e);
	ui->mem->connect_emu(&e);

	// Re-home the debugger panels into dock widgets arranged around the
	// control panel. The control panel is the permanent center - it IS the
	// machine - while the debugger modules dock around it and can be moved,
	// floated, tabbed or hidden. Reparenting the existing group boxes via
	// setWidget() keeps all their child pointers (ui->dasm, ui->r0, ...) and
	// signal wiring intact; only their host changes.
	dock_regs = new QDockWidget(tr("Registers"), this);
	dock_dasm = new QDockWidget(tr("Disassembly"), this);
	dock_mem  = new QDockWidget(tr("Memory"), this);

	dock_regs->setObjectName("dock_regs");
	dock_dasm->setObjectName("dock_dasm");
	dock_mem->setObjectName("dock_mem");

	// dock title bars now carry the names; drop the redundant group titles
	ui->group_registers->setTitle("");
	ui->group_dasm->setTitle("");
	ui->group_mem->setTitle("");

	dock_regs->setWidget(ui->group_registers);
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
	ui->menuView->addAction(dock_regs->toggleViewAction());
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

	// connect register edits
	for (int i=EM400_REG_R0 ; i<EM400_REG_COUNT ; i++) {
		if (r[i]) connect(r[i], &QSpinBox::editingFinished, [=](){ e.set_reg(i, r[i]->value()); });
	}

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
	addDockWidget(Qt::RightDockWidgetArea, dock_regs);  // small modules, stacked

	for (QDockWidget *d : {dock_dasm, dock_mem, dock_regs}) {
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
	bool any = !dock_dasm->isHidden() || !dock_mem->isHidden() || !dock_regs->isHidden();
	QSignalBlocker block(ui->actionDebugger);
	ui->actionDebugger->setChecked(any);
}

// -----------------------------------------------------------------------
void MainWindow::disable_widgets(bool state)
{
	for (int i=EM400_REG_R0 ; i<EM400_REG_COUNT ; i++) {
		if (i == EM400_REG_KB) continue; // keys are always enabled
		if (r[i]) r[i]->setDisabled(state);
	}
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
	if (!r[reg]) return;

	// update all register values
	r[reg]->setValue(val);

	slot_dasm_update();

	// do register-specific things
	if (reg <= EM400_REG_R7) {
		if (reg == EM400_REG_R0) update_r0_status(val);
	} else if (reg == EM400_REG_IC) {
		slot_dasm_update();
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
	dock_regs->setVisible(state);
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

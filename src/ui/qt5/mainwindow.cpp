#include <QCloseEvent>
#include <QTimer>
#include <QDebug>
#include <QFileDialog>
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
	slot_debugger_enabled_changed(false);
}

// -----------------------------------------------------------------------
MainWindow::~MainWindow()
{
	delete ui;
}

// -----------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent* event)
{
	e.stop();
	event->accept();
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

	// TODO: where to update memory view?
	ui->mem->update_contents(0, 0);
	// TODO: where/how to force-update everything?
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
	ui->group_dasm->setVisible(state);
	ui->group_registers->setVisible(state);
	ui->group_mem->setVisible(state);
	//ui->statusbar->setVisible(state);
	for (int i=0 ; i<10 ; i++) qApp->processEvents(); // StackOverflow, I don't even...
	adjustSize();
}

// -----------------------------------------------------------------------
void MainWindow::slot_smallcp_changed(bool state)
{
	ui->cp->slot_small_panel_changed(state);
	for (int i=0 ; i<10 ; i++) qApp->processEvents(); // StackOverflow, I don't even...
	adjustSize();
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

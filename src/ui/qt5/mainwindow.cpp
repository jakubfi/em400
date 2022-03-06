#include <QCloseEvent>
#include <QTimer>
#include <QDebug>
#include <QFileDialog>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "emdas.h"

// -----------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	r[ECTL_REG_R0] = ui->r0;
	r[ECTL_REG_R1] = ui->r1;
	r[ECTL_REG_R2] = ui->r2;
	r[ECTL_REG_R3] = ui->r3;
	r[ECTL_REG_R4] = ui->r4;
	r[ECTL_REG_R5] = ui->r5;
	r[ECTL_REG_R6] = ui->r6;
	r[ECTL_REG_R7] = ui->r7;

	r[ECTL_REG_IC] = ui->ic;
	r[ECTL_REG_AC] = ui->ac;
	r[ECTL_REG_AR] = ui->ar;
	r[ECTL_REG_IR] = ui->ir;
	r[ECTL_REG_SR] = ui->sr;
	r[ECTL_REG_RZ] = ui->rz;
	r[ECTL_REG_KB] = ui->kb;
	r[ECTL_REG_MC] = ui->mc;

	r2[ECTL_REG_R0] = ui->r0_2;
	r2[ECTL_REG_R1] = ui->r1_2;
	r2[ECTL_REG_R2] = ui->r2_2;
	r2[ECTL_REG_R3] = ui->r3_2;
	r2[ECTL_REG_R4] = ui->r4_2;
	r2[ECTL_REG_R5] = ui->r5_2;
	r2[ECTL_REG_R6] = ui->r6_2;
	r2[ECTL_REG_R7] = ui->r7_2;

	ui->dasm->connect_emu(&e);

	// create toolbar
	load = ui->toolBar->addAction(QIcon(":/icons/open.svg"), "Load OS image");
	ui->toolBar->addSeparator();
	start = ui->toolBar->addAction(QIcon(":/icons/start.svg"), "Start", this, &MainWindow::start_clicked);
	stop = ui->toolBar->addAction(QIcon(":/icons/stop.svg"), "Stop", this, &MainWindow::stop_clicked);
	cycle = ui->toolBar->addAction(QIcon(":/icons/cycle.svg"), "Cycle", this, &MainWindow::cycle_clicked);
	ui->toolBar->addSeparator();
	bin = ui->toolBar->addAction(QIcon(":/icons/bin.svg"), "Binary load (BIN)");
	clock = ui->toolBar->addAction(QIcon(":/icons/clock.svg"), "Clock", this, &MainWindow::clock_clicked);
	clock->setCheckable(true);
	clock->setChecked(e.get_clock());
	ui->toolBar->addAction(QIcon(":/icons/oprq.svg"), "Operator request (OPRQ)", this, &MainWindow::oprq_clicked);
	ui->toolBar->addSeparator();
	ui->toolBar->addAction(QIcon(":/icons/clear.svg"), "Clear", this, &MainWindow::clear_clicked);

	// connect signals from EmuModel
	connect(&e, &EmuModel::cpu_state_changed, this, &MainWindow::cpu_state_changed);
	connect(&e, &EmuModel::cpu_reg_changed, this, &MainWindow::cpu_reg_changed);
	connect(&e, &EmuModel::cpu_ips_tick, this, &MainWindow::cpu_ips_update);
	connect(&e, &EmuModel::cpu_alarm, this, &MainWindow::cpu_alarm_update);
	connect(&e, &EmuModel::cpu_p, this, &MainWindow::cpu_p_update);

	// connect menu actions
	connect(ui->actionLoad_OS_image, &QAction::triggered, this, &MainWindow::load_os_image);
	connect(load, &QAction::triggered, this, &MainWindow::load_os_image);

	// connect register edits
	for (int i=ECTL_REG_R0 ; i<ECTL_REG_COUNT ; i++) {
		if (r[i]) connect(r[i], &QSpinBox::editingFinished, [=](){ e.set_reg(i, r[i]->value()); });
		if ((i <= ECTL_REG_R7) && r2[i]) connect(r2[i], &QSpinBox::editingFinished, [=](){ e.set_reg(i, r2[i]->value()); });
	}

	// create status bar
	cpu_state = new QLabel();
	cpu_state->setStyleSheet("font-weight: bold;");
	ui->statusbar->addPermanentWidget(cpu_state);

	p = new QLabel("P");
	p->setStyleSheet("font-weight: bold; color: #d57500;");
	ui->statusbar->addPermanentWidget(p);

	alarm = new QLabel("ALARM");
	alarm->setStyleSheet("font-weight: bold; color: red;");
	ui->statusbar->addPermanentWidget(alarm);

	ips = new QLabel();
	ui->statusbar->addPermanentWidget(ips);

	QFont font("Monospace");

	QLabel *flags_label = new QLabel("<span>Flags:</span>");
	flags_label->setFont(font);
	ui->statusbar->addWidget(flags_label);
	flags = new QLabel();
	alarm->setStyleSheet("font-weight: normal;");
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

	e.enable(60);
}

// -----------------------------------------------------------------------
MainWindow::~MainWindow()
{
	delete cpu_state_label;
	delete cpu_state;
	delete ui;
}

// -----------------------------------------------------------------------
void MainWindow::start_clicked()
{
	e.start();
}

// -----------------------------------------------------------------------
void MainWindow::stop_clicked()
{
	e.stop();
}

// -----------------------------------------------------------------------
void MainWindow::cycle_clicked()
{
	e.cycle();
}

// -----------------------------------------------------------------------
void MainWindow::clear_clicked()
{
	e.clear();
}

// -----------------------------------------------------------------------
void MainWindow::clock_clicked()
{
	e.set_clock(clock->isChecked());
}

// -----------------------------------------------------------------------
void MainWindow::oprq_clicked()
{
	e.oprq();
}

// -----------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent* event)
{
	e.off();
	event->accept();
}

// -----------------------------------------------------------------------
void MainWindow::disable_widgets(bool state)
{
	start->setDisabled(state);
	stop->setDisabled(!state);
	cycle->setDisabled(state);
	bin->setDisabled(state);

	for (int i=ECTL_REG_R0 ; i<ECTL_REG_COUNT ; i++) {
		if (i == ECTL_REG_KB) continue; // keys are always enabled
		if (r[i]) r[i]->setDisabled(state);
		if ((i <= ECTL_REG_R7) && r2[i]) r2[i]->setDisabled(state);
	}
}

// -----------------------------------------------------------------------
void MainWindow::cpu_state_changed(int state)
{
	QString named_state;
	unsigned s = e.get_state_simplified();
	if (s == ECTL_STATE_STOP) {
		named_state = "STOP";
		cpu_state->setStyleSheet("font-weight: bold; color: black;");
	} else if (s == ECTL_STATE_RUN) {
		named_state = "RUN";
		cpu_state->setStyleSheet("font-weight: bold; color: green;");
	} else {
		named_state = "WAIT";
		cpu_state->setStyleSheet("font-weight: bold; color: #d57500;");
	}

	cpu_state->setText(named_state);

	disable_widgets(state==ECTL_STATE_RUN);
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
void MainWindow::cpu_reg_changed(int reg, uint16_t val)
{
	if (!r[reg]) return;

	// update all register values
	r[reg]->setValue(val);

	// TODO: where to update memory view?
	ui->mem->update_contents(0, 0);

	// do register-specific things
	if (reg <= ECTL_REG_R7) {
		if (reg == ECTL_REG_R0) update_r0_status(val);
		r2[reg]->setValue((int16_t)val);
	} else if (reg == ECTL_REG_IC) {
		update_dasm_view();
	} else if (reg == ECTL_REG_SR) {
		update_sr_status(val);
	}
}

// -----------------------------------------------------------------------
void MainWindow::update_dasm_view()
{
	int nb = e.get_reg(ECTL_REG_Q) * e.get_reg(ECTL_REG_NB);
	int ic = e.get_reg(ECTL_REG_IC);
	ui->dasm->update_contents(nb, ic);
}

// -----------------------------------------------------------------------
void MainWindow::cpu_ips_update(unsigned long ips)
{
	QString mips_t = QString("%1 MIPS").arg(QString::number(ips/1000000.0, 'f', 3));
	this->ips->setText(mips_t);
}

// -----------------------------------------------------------------------
void MainWindow::cpu_alarm_update(bool state)
{
	alarm->setHidden(!state);
}

// -----------------------------------------------------------------------
void MainWindow::cpu_p_update(bool state)
{
	p->setHidden(!state);
}

// -----------------------------------------------------------------------
void MainWindow::load_os_image()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Open OS image..."), nullptr, nullptr);
	if (filename.isNull()) {
			return;
	}

	QFileInfo fi(filename);

	e.load(filename);

}

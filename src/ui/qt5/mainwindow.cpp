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

	connect(&e, &EmuModel::cpu_state_changed, this, &MainWindow::cpu_state_changed);
	connect(&e, &EmuModel::cpu_reg_changed, this, &MainWindow::cpu_reg_changed);

	// menu actions
	connect(ui->actionLoad_OS_image, &QAction::triggered, this, &MainWindow::load_os_image);
	connect(load, &QAction::triggered, this, &MainWindow::load_os_image);

	// create status bar
	cpu_state_label = new QLabel("CPU state:");
	ui->statusbar->addWidget(cpu_state_label);

	cpu_state = new QLabel("STOP");
	cpu_state->setStyleSheet("font-weight: bold");
	ui->statusbar->addWidget(cpu_state);

	alarm = new QLabel("");
	alarm->setStyleSheet("font-weight: bold; color: red");
	ui->statusbar->addWidget(alarm);


	e.enable();
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
void MainWindow::on_ic_editingFinished()
{
	e.set_reg(ECTL_REG_IC, ui->ic->value());
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

	ui->ic->setDisabled(state);
	ui->ir->setDisabled(state);
	ui->sr->setDisabled(state);
	ui->kb->setDisabled(state);
	ui->rz->setDisabled(state);
	ui->mc->setDisabled(state);
	ui->ac->setDisabled(state);
	ui->ar->setDisabled(state);

	ui->r0->setDisabled(state);
	ui->r1->setDisabled(state);
	ui->r2->setDisabled(state);
	ui->r3->setDisabled(state);
	ui->r4->setDisabled(state);
	ui->r5->setDisabled(state);
	ui->r6->setDisabled(state);
	ui->r7->setDisabled(state);

	ui->r0_2->setDisabled(state);
	ui->r1_2->setDisabled(state);
	ui->r2_2->setDisabled(state);
	ui->r3_2->setDisabled(state);
	ui->r4_2->setDisabled(state);
	ui->r5_2->setDisabled(state);
	ui->r6_2->setDisabled(state);
	ui->r7_2->setDisabled(state);

}

// -----------------------------------------------------------------------
void MainWindow::cpu_state_changed(int state)
{
	QString named_state;
	unsigned s = e.get_state_simplified();
	if (s == ECTL_STATE_STOP) {
		named_state = "STOP";
		cpu_state->setStyleSheet("font-weight: bold; color: black");
	} else if (s == ECTL_STATE_RUN) {
		named_state = "RUN";
		cpu_state->setStyleSheet("font-weight: bold; color: green");
	} else {
		named_state = "WAIT";
		cpu_state->setStyleSheet("font-weight: bold; color: #d57500");
	}

	cpu_state->setText(named_state);

	disable_widgets(state==ECTL_STATE_RUN);
}

// -----------------------------------------------------------------------
void MainWindow::cpu_reg_changed(int reg)
{
	if (!r[reg]) return;

	r[reg]->setValue(e.get_reg(reg));
	ui->mem->update_contents(0, 0);

	if (reg <= ECTL_REG_R7) {
		r2[reg]->setValue((int16_t)e.get_reg(reg));
	} else if (reg == ECTL_REG_IC) {
		update_dasm_view();
	} else if (reg == ECTL_REG_ALARM) {
		if (e.get_reg(ECTL_REG_ALARM)) {
			alarm->setText("ALARM");
		} else {
			alarm->setText("");
		}
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
void MainWindow::load_os_image()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Open OS image..."), nullptr, nullptr);
	if (filename.isNull()) {
			return;
	}

	QFileInfo fi(filename);

	e.load(filename);

}

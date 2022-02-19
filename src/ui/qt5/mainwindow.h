#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QSpinBox>
#include <emdas.h>
#include "emumodel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private:
	EmuModel e;
	Ui::MainWindow *ui;

	QLabel *cpu_state;
	QLabel *cpu_state_label;
	QLabel *alarm;
	QLabel *ips;

	QAction *load, *start, *stop, *cycle, *bin, *clock;
	QSpinBox *r[ECTL_REG_COUNT];
	QSpinBox *r2[ECTL_REG_R7+1];

	void disable_widgets(bool disabled);

private slots:
	void start_clicked();
	void stop_clicked();
	void cycle_clicked();
	void clear_clicked();
	void clock_clicked();
	void oprq_clicked();
	void on_ic_editingFinished();
	void closeEvent(QCloseEvent* event);
	void update_dasm_view();
	void cpu_state_changed(int state);
	void cpu_reg_changed(int reg);
	void cpu_ips_update(unsigned long ips);
	void cpu_alarm_update(bool a);

	void load_os_image();
};

#endif // MAINWINDOW_H

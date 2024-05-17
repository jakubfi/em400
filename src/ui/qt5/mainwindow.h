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
	QLabel *q, *bs, *nb, *flags, *p;

	QAction *load, *start, *stop, *cycle, *bin, *clock;
	QSpinBox *r[ECTL_REG_COUNT] = { nullptr };
	QSpinBox *r2[ECTL_REG_R7+1] = { nullptr };

	void disable_widgets(bool disabled);
	void update_sr_status(uint16_t val);
	void update_r0_status(uint16_t r0);

private slots:
	void closeEvent(QCloseEvent* event);
	void update_dasm_view();
    void cpu_reg_changed(int reg, uint16_t val);
	void cpu_ips_update(unsigned long ips);

	void load_os_image();
};

#endif // MAINWINDOW_H

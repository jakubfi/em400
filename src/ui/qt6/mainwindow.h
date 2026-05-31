#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QSpinBox>
#include <QDockWidget>
#include <emdas.h>
#include "emumodel.h"
#include "regview.h"
#include "intview.h"

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

	QLabel *ips;
	QLabel *q, *bs, *nb, *flags, *mc;

	QDockWidget *dock_uregs, *dock_sregs, *dock_dasm, *dock_mem, *dock_ints;

	RegCompact *uregs, *sregs;
	IntView *ints;

	void update_mc_status(bool mc);
	void update_sr_status(uint16_t val);
	void update_r0_status(uint16_t r0);

	void apply_default_layout();
	void sync_debugger_action();

private slots:
	void closeEvent(QCloseEvent* event);
	void slot_dasm_update();
	void slot_cpu_reg_changed(int reg, uint16_t val);
	void slot_ips_update(unsigned long ips);

	void load_os_image();

public slots:
	void slot_debugger_enabled_changed(bool state);
	void slot_smallcp_changed(bool state);
	void slot_binary_key_toggled(bool state);

};

#endif // MAINWINDOW_H

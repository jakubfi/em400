#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QSpinBox>
#include <QDockWidget>
#include <QList>
#include <emdas.h>
#include "emumodel.h"
#include "dasmview.h"
#include "memview.h"
#include "regview.h"
#include "intview.h"
#include "mapview.h"
#include "brkview.h"
#include "watchview.h"
#include "stackview.h"

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
	QLabel *edit_mode;
	QWidget *edit_box;

	// All debugger docks, in View-menu order. Built by register_dock(); the
	// named handles below alias entries here and exist only for the bespoke
	// default arrangement in apply_default_layout().
	QList<QDockWidget *> docks;
	QDockWidget *dock_uregs, *dock_sregs, *dock_dasm, *dock_mem, *dock_ints, *dock_map, *dock_brk, *dock_watch, *dock_stack;

	DasmView *dasm;
	MemView *mem;
	RegCompact *uregs, *sregs;
	IntView *ints;
	MapView *map;
	BrkView *brk;
	WatchView *watch;
	StackView *stack;

	void update_mc_status(int mc);
	void update_sr_status(uint16_t val);
	void update_r0_status(uint16_t r0);

	QDockWidget *register_dock(QWidget *view, const QString &title, const QString &objname);
	void apply_default_layout();
	void sync_debugger_action();

private slots:
	void closeEvent(QCloseEvent* event);
	void slot_dasm_update();
	void slot_cpu_reg_changed(int reg, uint16_t val);
	void slot_ips_update(unsigned long ips);
	void slot_edit_mode_changed(bool editing, bool insert);

	void load_os_image();

public slots:
	void slot_debugger_enabled_changed(bool state);
	void slot_smallcp_changed(bool state);
	void slot_panel_theme_changed(bool state);

};

#endif // MAINWINDOW_H

// vim: tabstop=4 shiftwidth=4 autoindent

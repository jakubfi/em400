#ifndef EMUMODEL_H
#define EMUMODEL_H

#include <QObject>
#include <QTimer>
#include "ectl.h"
#include <emdas.h>

class EmuModel : public QObject
{
	Q_OBJECT

public:
	EmuModel();
	~EmuModel();

	void enable() { timer.start(); }
	void disable() { timer.stop(); }
	void set_refresh_rate(int hz);
	int get_state_simplified();
	int get_reg(int i) { return r[i]; }
	int get_state() { return cpu_state; }
	int get_mem(int nb, int addr);
	int get_mem(int nb, int addr, uint16_t *m, int count);
	void start();
	void stop();
	void clear();
	void cycle();
	void set_clock(bool state);
	bool get_clock();
	void oprq();
	void set_reg(int i, int v);
	void off();
	bool load(QString filename);

private:
	QTimer timer;
	QTimer ips_timer;
	int cpu_state;
	int r[ECTL_REG_COUNT];

	void check_state();
	void check_regs();

private slots:
	void on_timer_timeout();
	void on_ips_timer_timeout();

signals:
	void cpu_state_changed(int state);
	void cpu_reg_changed(int reg);
	void cpu_ips_tick(unsigned long ips);
	void cpu_alarm(bool alarm);

};

#endif // EMUMODEL_H

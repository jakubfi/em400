#ifndef EMUMODEL_H
#define EMUMODEL_H

#include <QObject>
#include <QTimer>
#include "libem400.h"
#include <emdas.h>

class EmuModel : public QObject
{
	Q_OBJECT

public:
	EmuModel();
	~EmuModel();

	void run();
	void stop();
	bool load_os_image(QString filename);

	void set_reg(int i, int v) { em400_reg_set(i, v); }
	void set_kb(uint16_t v) { em400_cp_kb(v); }
	int get_reg(int i) { return last_reg[i]; }
	int get_mem(int nb, int addr);
	int get_mem(int nb, int addr, uint16_t *m, int count);
	bool get_p() { return last_p; }
	bool get_mc() { return last_mc; }
	bool get_alarm() { return last_alarm; }
	int get_nb() { return em400_nb(); }
	int get_qnb() { return em400_qnb(); }
	bool get_clock() { return em400_cp_clock_led(); }

private:
	QTimer timer_realtime;
	QTimer timer_slow;
	QTimer timer_ips;

	int last_cpu_state;
	int last_reg[EM400_REG_COUNT];
	uint16_t last_bus_w;
	bool last_clock;
	bool last_alarm, last_p, last_mc;

	void sync_state(bool force=false);
	void sync_regs(bool force=false);
	void sync_bus_w(bool force=false);
	void sync_clock(bool force=false);
	void sync_flags(bool force=false);
	void sync_ips();

private slots:
	void on_timer_realtime_timeout();
	void on_timer_slow_timeout();
	void on_timer_ips_timeout();

public slots:
	void slot_cpu_start(bool state) { em400_cp_start(state); }
	void slot_clear() { em400_cp_clear(); }
	void slot_cycle() { em400_cp_cycle(); }
	void slot_clock_enabled(bool state) { em400_cp_clock(state); }
	void slot_oprq() { em400_cp_oprq(); }
	void slot_reg_select(int r) { em400_cp_reg_select(r); }
	void slot_load() { em400_cp_load(); }
	void slot_bin() { em400_cp_bin(); }
	void slot_fetch() { em400_cp_fetch(); }
	void slot_store() { em400_cp_store(); }

signals:
	void signal_state_changed(int state);
	void signal_bus_w_changed(uint16_t val);
	void signal_reg_changed(int reg, uint16_t val);
	void signal_cpu_ips_tick(unsigned long ips);
	void signal_alarm_changed(bool alarm);
	void signal_p_changed(bool p);
	void signal_mc_changed(bool mc);
	void signal_clock_changed(bool clock);
};

#endif // EMUMODEL_H

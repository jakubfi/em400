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
	bool set_mem(int nb, int addr, uint16_t v) { return em400_mem_write(nb, addr, &v, 1); }
	// Read P live (like get_clock), not from last_p: last_p is sampled by the
	// realtime timer, independently of the registers, so using it for the dasm
	// "skip" bar can paint a stale-red frame before sync_flags catches up.
	bool get_p() { return em400_cp_p_led(); }
	int get_mc() { return last_mc; }
	bool get_alarm() { return last_alarm; }
	int get_nb() { return em400_nb(); }
	int get_qnb() { return em400_qnb(); }
	bool get_clock() { return em400_cp_clock_led(); }
	uint32_t get_rz() { return last_rz; }
	int get_mem_map(int seg) { return em400_mem_map(seg); }

	// interrupt view helpers (structural, language-free)
	int int_mask_bit(int n) { return em400_int_mask_bit(n); }
	void set_int(int n, bool on) { if (on) em400_int_set(n); else em400_int_clear(n); }

private:
	QTimer timer_realtime;
	QTimer timer_slow;
	QTimer timer_ips;

	int last_cpu_state;
	int last_reg[EM400_REG_COUNT] = {0};
	uint16_t last_bus_w;
	uint32_t last_rz = 0;
	int last_map[16] = {0};
	bool last_clock;
	bool last_alarm, last_p;
	int last_mc;

	void sync_state(bool force=false);
	void sync_regs(bool force=false);
	void sync_rz(bool force=false);
	void sync_map(bool force=false);
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
	void signal_rz_changed(uint32_t val);
	void signal_mem_map_changed(int seg, uint16_t map);
	void signal_cpu_ips_tick(unsigned long ips);
	void signal_alarm_changed(bool alarm);
	void signal_p_changed(bool p);
	void signal_mc_changed(int mc);
	void signal_clock_changed(bool clock);
};

#endif // EMUMODEL_H

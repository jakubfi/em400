#ifndef EMUMODEL_H
#define EMUMODEL_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include <QString>
#include "libem400.h"
#include <emdas.h>

struct BrkInfo {
	unsigned id;
	QString expr;
	bool enabled;
};

struct WatchInfo {
	unsigned id;
	QString expr;
};

class EmuModel : public QObject
{
	Q_OBJECT

public:
	EmuModel();
	~EmuModel();

	void run();
	void stop();
	bool load_os_image(QString filename);

	bool is_powered() { return em400_is_powered(); }

	void set_reg(int i, int v) { em400_reg_set(i, v); }
	void set_kb(uint16_t v) { em400_cp_kb(v); }
	int get_reg(int i) { return last_reg[i]; }
	int get_mem(int nb, int addr);
	int get_mem(int nb, int addr, uint16_t *m, int count);
	bool set_mem(int nb, int addr, uint16_t v) { return em400_mem_write(nb, addr, &v, 1); }
	// Read P live, not from last_p: last_p is sampled by the realtime timer,
	// independently of the registers, so using it for the dasm "skip" bar can
	// paint a stale-red frame before sync_flags catches up.
	bool get_p() { return em400_cp_p_led(); }
	int get_mc() { return last_mc; }
	bool get_alarm() { return last_alarm; }
	int get_nb() { return em400_nb(); }
	int get_qnb() { return em400_qnb(); }
	uint32_t get_rz() { return last_rz; }
	int get_mem_map(int seg) { return em400_mem_map(seg); }

	// live-swap a removable drive: (re)load or eject the device at this address,
	// no-op unless powered and the device permits it. Config side is ConfigController's.
	void reload_media(unsigned chan, unsigned dev, unsigned slot, const char *path);

	// host-side buzzer volume; the core just stores it atomically, so it is safe
	// to push whether or not the machine is powered. Config side is ConfigController's.
	void set_volume(int v) { em400_volume_pct_set(v); }

	// interrupt view helpers (structural, language-free). The mask topology is
	// fixed hardware wiring (a const table), so it is not power-gated: gating it
	// would collapse every interrupt onto mask bit 0 and break the grouping.
	int int_mask_bit(int n) { return em400_int_mask_bit(n); }
	void set_int(int n, bool on) { if (on) em400_int_set(n); else em400_int_clear(n); }

	// breakpoint editing/listing API. The list only ever changes through these
	// UI-thread calls, so the view rebuilds from brk_list() right after each
	// edit instead of polling. brk_add returns the new id (>=0) or -1 and fills
	// `err` with the parser message on failure.
	QVector<BrkInfo> brk_list();
	int brk_add(const QString &expr, QString &err);
	int brk_edit(unsigned id, const QString &expr, QString &err);
	void brk_del(unsigned id) { em400_brk_delete(id); }
	void brk_set_enabled(unsigned id, bool en) { em400_brk_enable(id, en); }

	// watch listing/editing API, same UI-thread-only contract as the breakpoint
	// API above: the list changes only through these calls, so the view rebuilds
	// from watch_list() after each structural edit. watch_edit keeps the id (the
	// core has a real edit op), and watch_eval evaluates one watch on demand to
	// fill the value column. watch_add/watch_edit return 0/-1 and fill `err`.
	QVector<WatchInfo> watch_list();
	int watch_add(const QString &expr, QString &err);
	int watch_edit(unsigned id, const QString &expr, QString &err);
	void watch_del(unsigned id) { em400_watch_delete(id); }
	bool watch_eval(unsigned id, int &value, QString &err);

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
	int last_brk_hit = -1;

	void sync_state(bool force=false);
	void sync_regs(bool force=false);
	void sync_rz(bool force=false);
	void sync_map(bool force=false);
	void sync_bus_w(bool force=false);
	void sync_clock(bool force=false);
	void sync_flags(bool force=false);
	void sync_brk_hit(bool force=false);
	void sync_ips();
	void sync_all(bool force=false);

private slots:
	void on_timer_realtime_timeout();
	void on_timer_slow_timeout();
	void on_timer_ips_timeout();

public slots:
	// the ignition switch drives this; on->em400_init()+run(), off->stop()+
	// em400_shutdown(), then blank the views so the panel reads as dead
	void slot_power(bool on);

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
	// power lifecycle flipped (after init/shutdown settled); config gating listens
	void signal_power_changed(bool on);
	// id of the breakpoint that stopped the machine, or -1 when none (running,
	// or stopped by something other than a breakpoint).
	void signal_brk_hit_changed(int id);
	// the breakpoint list changed structurally through an edit call (e.g. an add
	// from the dasm view); listeners rebuild from brk_list().
	void signal_brk_list_changed();
};

#endif // EMUMODEL_H

// vim: tabstop=4 shiftwidth=4 autoindent

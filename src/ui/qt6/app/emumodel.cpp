#include <QtDebug>
#include <QFile>
#include <cstdlib>
#include "emumodel.h"
#include "libem400.h"


// -----------------------------------------------------------------------
EmuModel::EmuModel()
{
	timer_realtime.setInterval(1000.0f / 60);
	timer_slow.setInterval(1000.0f / 20);
	timer_ips.setInterval(1000.0f / 2);
	connect(&timer_realtime, &QTimer::timeout, this, &EmuModel::on_timer_realtime_timeout);
	connect(&timer_slow, &QTimer::timeout, this, &EmuModel::on_timer_slow_timeout);
	connect(&timer_ips, &QTimer::timeout, this, &EmuModel::sync_ips);
}

// -----------------------------------------------------------------------
EmuModel::~EmuModel()
{
	stop();
}

// -----------------------------------------------------------------------
void EmuModel::run()
{
	sync_bus_w(true);
	sync_state(true);
	sync_flags(true);
	sync_regs(true);
	sync_rz(true);
	sync_map(true);
	sync_clock(true);
	sync_brk_hit(true);
	sync_ips();

	timer_realtime.start();
	timer_slow.start();
	timer_ips.start();
}

// -----------------------------------------------------------------------
void EmuModel::stop()
{
	timer_realtime.stop();
	timer_slow.stop();
	timer_ips.stop();
}

// -----------------------------------------------------------------------
void EmuModel::on_timer_realtime_timeout()
{
	sync_bus_w();
	sync_flags();
	sync_state();
}

// -----------------------------------------------------------------------
void EmuModel::on_timer_slow_timeout()
{
	sync_regs();
	sync_rz();
	sync_map();
	sync_clock();
	sync_brk_hit();
}

// -----------------------------------------------------------------------
void EmuModel::on_timer_ips_timeout()
{
	sync_ips();
}

// -----------------------------------------------------------------------
void EmuModel::sync_state(bool force)
{
	int state = em400_cpu_state();
	if (force || (state != last_cpu_state)) {
		last_cpu_state = state;
		if ((state != EM400_STATE_RUN) && (state != EM400_STATE_WAIT)) {
			state = EM400_STATE_STOP;
		}
		emit signal_state_changed(state);
	}
}

// -----------------------------------------------------------------------
void EmuModel::sync_bus_w(bool force)
{
	uint16_t bus_w = em400_cp_w_leds();
	if (force || (bus_w != last_bus_w)) {
		last_bus_w = bus_w;
		emit signal_bus_w_changed(bus_w);
	}
}

// -----------------------------------------------------------------------
void EmuModel::sync_flags(bool force)
{
	bool alarm = em400_cp_alarm_led();
	bool p = em400_cp_p_led();
	int mc = em400_mc();

	if (force || (alarm != last_alarm)) {
		last_alarm = alarm;
		emit signal_alarm_changed(alarm);
	}
	if (force || (p != last_p)) {
		last_p = p;
		emit signal_p_changed(p);
	}
	if (force || (mc != last_mc)) {
		last_mc = mc;
		emit signal_mc_changed(mc);
	}
}

// -----------------------------------------------------------------------
void EmuModel::sync_regs(bool force)
{
	for (int i=EM400_REG_R0 ; i<EM400_REG_COUNT ; i++) {
		int reg = em400_reg(i);
		if (force || (reg != last_reg[i])) {
			last_reg[i] = reg;
			emit signal_reg_changed(i, reg);
		}
	}
}

// -----------------------------------------------------------------------
void EmuModel::sync_rz(bool force)
{
	uint32_t rz = em400_rz32();
	if (force || (rz != last_rz)) {
		last_rz = rz;
		emit signal_rz_changed(rz);
	}
}

// -----------------------------------------------------------------------
void EmuModel::sync_map(bool force)
{
	for (int seg=0 ; seg<16 ; seg++) {
		int map = em400_mem_map(seg);
		if (force || (map != last_map[seg])) {
			last_map[seg] = map;
			emit signal_mem_map_changed(seg, map);
		}
	}
}

// -----------------------------------------------------------------------
void EmuModel::sync_clock(bool force)
{
	bool clock = em400_cp_clock_led();
	if (force || (clock != last_clock)) {
		emit signal_clock_changed(clock);
		last_clock = clock;
	}
}

// -----------------------------------------------------------------------
// em400_brk_hit() is a cheap atomic read; the value only changes when the
// machine stops on a breakpoint or resumes (cp.c clears it on start/cycle/
// clear), so polling it on the slow timer is enough to drive the hit LED.
void EmuModel::sync_brk_hit(bool force)
{
	int hit = em400_brk_hit();
	if (force || (hit != last_brk_hit)) {
		emit signal_brk_hit_changed(hit);
		last_brk_hit = hit;
	}
}

// -----------------------------------------------------------------------
void EmuModel::sync_ips()
{
	emit signal_cpu_ips_tick(em400_ips_get());
}

// -----------------------------------------------------------------------
QVector<BrkInfo> EmuModel::brk_list()
{
	QVector<BrkInfo> out;
	em400_brk_foreach([](unsigned id, const char *expr, bool enabled, void *ctx) {
		static_cast<QVector<BrkInfo>*>(ctx)->append(
			BrkInfo{id, QString::fromUtf8(expr), enabled});
	}, &out);
	return out;
}

// -----------------------------------------------------------------------
int EmuModel::brk_add(const QString &expr, QString &err)
{
	QByteArray ba = expr.toUtf8();
	char *err_msg = nullptr;
	int err_beg = 0, err_end = 0;
	int id = em400_brk_add(ba.data(), &err_msg, &err_beg, &err_end);
	if (id < 0 && err_msg) {
		err = QString::fromUtf8(err_msg);
	}
	free(err_msg); // strdup'd by the evaluator; NULL-safe
	if (id >= 0) emit signal_brk_list_changed();
	return id;
}

// -----------------------------------------------------------------------
QVector<WatchInfo> EmuModel::watch_list()
{
	QVector<WatchInfo> out;
	em400_watch_foreach([](unsigned id, const char *expr, void *ctx) {
		static_cast<QVector<WatchInfo>*>(ctx)->append(
			WatchInfo{id, QString::fromUtf8(expr)});
	}, &out);
	return out;
}

// -----------------------------------------------------------------------
int EmuModel::watch_add(const QString &expr, QString &err)
{
	QByteArray ba = expr.toUtf8();
	char *err_msg = nullptr;
	int err_beg = 0, err_end = 0;
	int id = em400_watch_add(ba.data(), &err_msg, &err_beg, &err_end);
	if (id < 0 && err_msg) {
		err = QString::fromUtf8(err_msg);
	}
	free(err_msg); // NULL-safe
	return id;
}

// -----------------------------------------------------------------------
int EmuModel::watch_edit(unsigned id, const QString &expr, QString &err)
{
	QByteArray ba = expr.toUtf8();
	char *err_msg = nullptr;
	int err_beg = 0, err_end = 0;
	int ret = em400_watch_edit(id, ba.data(), &err_msg, &err_beg, &err_end);
	if (ret < 0 && err_msg) {
		err = QString::fromUtf8(err_msg);
	}
	free(err_msg); // NULL-safe
	return ret;
}

// -----------------------------------------------------------------------
// Evaluate one watch for display. Returns true and sets `value` on success; on
// a runtime eval error (or unknown id) returns false and sets `err`. Like
// brk_eval, this re-parses on the UI thread and reads live machine state, so it
// is meant to be called while the machine is stopped.
bool EmuModel::watch_eval(unsigned id, int &value, QString &err)
{
	char *err_msg = nullptr;
	int err_beg = 0, err_end = 0, res = 0;
	if (em400_watch_eval(id, &res, &err_msg, &err_beg, &err_end) < 0) {
		err = QStringLiteral("?");
		return false; // no such watch; err_msg untouched (NULL)
	}
	if (res < 0) {
		err = err_msg ? QString::fromUtf8(err_msg) : QStringLiteral("error");
		free(err_msg);
		return false;
	}
	free(err_msg); // NULL-safe
	value = res;
	return true;
}

// -----------------------------------------------------------------------
int EmuModel::get_mem(int nb, int addr, uint16_t *m, int count)
{
	return em400_mem_read(nb, addr, m, count);
}

// -----------------------------------------------------------------------
int EmuModel::get_mem(int nb, int addr)
{
	uint16_t v;
	if (em400_mem_read(nb, addr, &v, 1)) {
		return v;
	} else {
		return -1;
	}
}

// -----------------------------------------------------------------------
bool EmuModel::load_os_image(QString filename)
{
	QFile file;
	file.setFileName(filename);

	if (!file.open(QIODevice::ReadOnly|QIODevice::ExistingOnly)) {
		return false;
	}

	FILE *f = fdopen(file.handle(), "r");
	em400_load_os_image(f);
	fclose(f);
	return true;
}

// vim: tabstop=4 shiftwidth=4 autoindent

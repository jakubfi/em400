#include <QtDebug>
#include <QFile>
#include "emumodel.h"
#include "ectl.h"


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
	sync_regs(true);
	sync_clock(true);
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
	ectl_cpu_off();
}

// -----------------------------------------------------------------------
void EmuModel::on_timer_realtime_timeout()
{
	sync_bus_w();
	sync_state();
}

// -----------------------------------------------------------------------
void EmuModel::on_timer_slow_timeout()
{
	sync_regs();
	sync_clock();
}

// -----------------------------------------------------------------------
void EmuModel::on_timer_ips_timeout()
{
	sync_ips();
}

// -----------------------------------------------------------------------
void EmuModel::sync_state(bool force)
{
	int state = ectl_cpu_state_get();
	if (force || (state != last_cpu_state)) {
		last_cpu_state = state;
		if ((state == ECTL_STATE_RUN) || (state == ECTL_STATE_CLM)) {
			state = ECTL_STATE_RUN;
		} else if (state == ECTL_STATE_WAIT) {
			state = ECTL_STATE_WAIT;
		} else {
			state = ECTL_STATE_STOP;
		}
		emit signal_state_changed(state);
	}
}

// -----------------------------------------------------------------------
void EmuModel::sync_bus_w(bool force)
{
	int bus_w = ectl_bus_w_get();
	if (force || (bus_w != last_bus_w)) {
		last_bus_w = bus_w;
		emit signal_bus_w_changed(bus_w);
	}
}

// -----------------------------------------------------------------------
void EmuModel::sync_regs(bool force)
{
	for (int i=ECTL_REG_R0 ; i<ECTL_REG_COUNT ; i++) {
		int reg = ectl_reg_get(i);

		if (force || (reg != last_reg[i])) {
			last_reg[i] = reg;
			if (i == ECTL_REG_ALARM) {
				emit signal_alarm_changed((bool)reg);
			} else if (i == ECTL_REG_P) {
				emit signal_p_changed((bool)reg);
			} else {
				emit signal_reg_changed(i, reg);
			}
		}
	}
}

// -----------------------------------------------------------------------
void EmuModel::sync_clock(bool force)
{
	int clock = ectl_clock_get();
	if (force || (clock != last_clock)) {
		emit signal_clock_changed(clock);
		last_clock = clock;
	}
}

// -----------------------------------------------------------------------
void EmuModel::sync_ips()
{
	emit signal_cpu_ips_tick(ectl_ips_get());
}

// -----------------------------------------------------------------------
void EmuModel::set_reg(int i, int v)
{
	ectl_reg_set(i, v);
}

// -----------------------------------------------------------------------
int EmuModel::get_mem(int nb, int addr, uint16_t *m, int count)
{
	return ectl_mem_read_n(nb, addr, m, count);
}

// -----------------------------------------------------------------------
int EmuModel::get_mem(int nb, int addr)
{
	uint16_t v;
	if (ectl_mem_read_n(nb, addr, &v, 1)) {
		return v;
	} else {
		return -1;
	}
}

// -----------------------------------------------------------------------
bool EmuModel::load(QString filename)
{
	QFile file;
	file.setFileName(filename);

	if (!file.open(QIODevice::ReadOnly|QIODevice::ExistingOnly)) {
		return false;
	}

	FILE *f = fdopen(file.handle(), "r");
	ectl_load(f, "name", 0, 0);
	fclose(f);
	return true;
}

// -----------------------------------------------------------------------
void EmuModel::slot_cpu_start(bool state)
{
	if (state) ectl_cpu_start();
	else ectl_cpu_stop();
}

// -----------------------------------------------------------------------
void EmuModel::slot_clear()
{
	ectl_cpu_clear();
}

// -----------------------------------------------------------------------
void EmuModel::slot_cycle()
{
	ectl_cpu_cycle();
}

// -----------------------------------------------------------------------
void EmuModel::slot_clock_enabled(bool state)
{
	ectl_clock_set(state);
}

// -----------------------------------------------------------------------
void EmuModel::slot_oprq()
{
	ectl_oprq();
}

// -----------------------------------------------------------------------
void EmuModel::slot_reg_select(int r)
{
	ectl_reg_select(r);
}

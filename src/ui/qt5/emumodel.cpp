#include <QtDebug>
#include <QFile>
#include "emumodel.h"
#include "ectl.h"


// -----------------------------------------------------------------------
EmuModel::EmuModel()
{
	ips_timer.setInterval(500);
	connect(&ips_timer, &QTimer::timeout, this, &EmuModel::on_ips_timer_timeout);
	connect(&timer, &QTimer::timeout, this, &EmuModel::on_timer_timeout);
}

// -----------------------------------------------------------------------
EmuModel::~EmuModel()
{
    stop();
}

// -----------------------------------------------------------------------
void EmuModel::run(int hz)
{
    sync(true);
	on_ips_timer_timeout();
	ips_timer.start();
    timer.setInterval(1000/hz);
    timer.start();
}

// -----------------------------------------------------------------------
void EmuModel::stop()
{
    ips_timer.stop();
    timer.stop();
    ectl_cpu_off();
}

// -----------------------------------------------------------------------
void EmuModel::on_timer_timeout()
{
    sync();
}

// -----------------------------------------------------------------------
void EmuModel::on_ips_timer_timeout()
{
    emit signal_cpu_ips_tick(ectl_ips_get());
}

// -----------------------------------------------------------------------
void EmuModel::set_reg(int i, int v)
{
	ectl_reg_set(i, v);
}

// -----------------------------------------------------------------------
void EmuModel::sync(bool force)
{
    sync_state(force);
    sync_bus_w(force);
    sync_regs(force);
    sync_clock(force);
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
                qDebug() << "ALARM: " << reg;
            } else if (i == ECTL_REG_P) {
                emit signal_p_changed((bool)reg);
                qDebug() << "P: " << reg;
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
        qDebug() << "clock get: " << clock;
    }
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
    sync(true);
	return true;
}

// -----------------------------------------------------------------------
void EmuModel::slot_cpu_state(bool state)
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
void EmuModel::slot_clock_state(bool state)
{
    ectl_clock_set(state);
    qDebug() << "clock set: " << state;
}

// -----------------------------------------------------------------------
void EmuModel::slot_oprq()
{
    ectl_oprq();
}

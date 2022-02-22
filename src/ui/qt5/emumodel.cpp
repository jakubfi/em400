#include <QtDebug>
#include <QFile>
#include "emumodel.h"
#include "ectl.h"


// -----------------------------------------------------------------------
EmuModel::EmuModel()
{
	for (int i=0 ; i<ECTL_REG_COUNT ; i++) {
		r[i] = 0;
		emit cpu_reg_changed(i);
	}
	cpu_state = ECTL_STATE_OFF;
	emit cpu_state_changed(cpu_state);
	emit cpu_alarm(false);
	emit cpu_ips_tick(0);

	set_refresh_rate(60);
	connect(&timer, &QTimer::timeout, this, &EmuModel::on_timer_timeout);

	ips_timer.setInterval(500);
	connect(&ips_timer, &QTimer::timeout, this, &EmuModel::on_ips_timer_timeout);
	ips_timer.start();

}

// -----------------------------------------------------------------------
EmuModel::~EmuModel()
{

}

// -----------------------------------------------------------------------
void EmuModel::set_refresh_rate(int hz)
{
	timer.setInterval(1000/hz);
}

// -----------------------------------------------------------------------
int EmuModel::get_state_simplified()
{
	int state = ectl_cpu_state_get();
	if ((state == ECTL_STATE_RUN) || (state == ECTL_STATE_CLM)) {
		return ECTL_STATE_RUN;
	} else if (state == ECTL_STATE_WAIT) {
		return ECTL_STATE_WAIT;
	} else {
		return ECTL_STATE_STOP;
	}
}

// -----------------------------------------------------------------------
void EmuModel::start()
{
	ectl_cpu_start();
}

// -----------------------------------------------------------------------
void EmuModel::stop()
{
	ectl_cpu_stop();
}

// -----------------------------------------------------------------------
void EmuModel::clear()
{
	ectl_cpu_clear();
}

// -----------------------------------------------------------------------
void EmuModel::cycle()
{
	ectl_cpu_cycle();
}

// -----------------------------------------------------------------------
void EmuModel::set_clock(bool state)
{
	ectl_clock_set(state);
}

// -----------------------------------------------------------------------
bool EmuModel::get_clock()
{
	return ectl_clock_get();
}

// -----------------------------------------------------------------------
void EmuModel::oprq()
{
	ectl_oprq();
}

// -----------------------------------------------------------------------
void EmuModel::set_reg(int i, int v)
{
	ectl_reg_set(i, v);
}

// -----------------------------------------------------------------------
void EmuModel::off()
{
	timer.stop();
	ectl_cpu_off();
}

// -----------------------------------------------------------------------
void EmuModel::check_state()
{
	int state = get_state_simplified();
	if (state != cpu_state) {
		cpu_state = state;
		emit cpu_state_changed(state);
	}
}

// -----------------------------------------------------------------------
void EmuModel::check_regs()
{
	for (int i=ECTL_REG_R0 ; i<ECTL_REG_COUNT ; i++) {
		int reg = ectl_reg_get(i);
		if (reg != r[i]) {
			r[i] = reg;
			if (i == ECTL_REG_ALARM) {
				emit cpu_alarm(r[i]);
			} else {
				emit cpu_reg_changed(i);
			}
		}
	}
}

// -----------------------------------------------------------------------
void EmuModel::on_timer_timeout()
{
	check_state();
	check_regs();
}

// -----------------------------------------------------------------------
void EmuModel::on_ips_timer_timeout()
{
	emit cpu_ips_tick(ectl_ips_get());
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

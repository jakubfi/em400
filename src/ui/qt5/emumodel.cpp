#include <QtDebug>
#include <QFile>
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
	em400_off();
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
	bool mc = em400_cp_mc_led();

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
void EmuModel::sync_clock(bool force)
{
	bool clock = em400_cp_clock_led();
	if (force || (clock != last_clock)) {
		emit signal_clock_changed(clock);
		last_clock = clock;
	}
}

// -----------------------------------------------------------------------
void EmuModel::sync_ips()
{
	emit signal_cpu_ips_tick(em400_ips_get());
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








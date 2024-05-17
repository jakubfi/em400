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

    void run(int hz);
    void stop();
    bool load(QString filename);

	void set_reg(int i, int v);
    int get_reg(int i) { return last_reg[i]; }
	int get_mem(int nb, int addr);
	int get_mem(int nb, int addr, uint16_t *m, int count);

private:
	QTimer timer;
	QTimer ips_timer;

    int last_cpu_state;
    int last_reg[ECTL_REG_COUNT];
    int last_bus_w;
    bool last_clock;

    void sync(bool force=false);
    void sync_state(bool force=false);
    void sync_regs(bool force=false);
    void sync_bus_w(bool force=false);
    void sync_clock(bool force=false);

private slots:
	void on_timer_timeout();
	void on_ips_timer_timeout();

public slots:
    void slot_cpu_state(bool state);
    void slot_clear();
    void slot_cycle();
    void slot_clock_state(bool state);
    void slot_oprq();

signals:
    void signal_state_changed(int state);
    void signal_bus_w_changed(uint16_t val);
    void signal_reg_changed(int reg, uint16_t val);
    void signal_cpu_ips_tick(unsigned long ips);
    void signal_alarm_changed(bool alarm);
    void signal_p_changed(bool p);
    void signal_clock_changed(bool clock);
};

#endif // EMUMODEL_H

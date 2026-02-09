#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QMouseEvent>
#include "switch.h"
#include "led.h"
#include "rotary.h"
#include "ignition.h"

struct sw_desc {
	QRect r;
	bool momentary;
	QString snd_on;
	QString snd_off;
};

#define LED_CNT 16+11
#define SW_CNT 16+12

enum sw_lower_id {SW_STEP=16, SW_MODE, SW_STOPN, SW_CYCLE, SW_LOAD, SW_STORE, SW_FETCH, SW_START, SW_BIN, SW_CLEAR, SW_CLOCK, SW_OPRQ};
enum sw_led_id {LED_MODE=16, LED_STOPN, LED_CLOCK, LED_Q, LED_P, LED_MC, LED_IRQ, LED_RUN, LED_WAIT, LED_ALARM, LED_ON};

class ControlPanel : public QWidget
{
	Q_OBJECT

private:
	QPixmap plane[16];
	int width, height;
	QRect crop;

	bool check_ignition(QPoint &m);
	void change_dimensions(const QRect &rect);

protected:
	void paintEvent(QPaintEvent *event);
	QSize sizeHint() const;
	QSize minimumSizeHint() const;

public:
	explicit ControlPanel(QWidget *parent = nullptr);
	~ControlPanel();
	QSizePolicy sizePolicy();

	Switch *sw[SW_CNT];
	LED *led[LED_CNT];
	Rotary *rotary;
	Ignition *ignition;

	void dim(float val);
	void set_volume(int volume_percent);

public slots:
	void slot_bus_w_changed(uint16_t val);
	void slot_state_changed(int state);
	void slot_small_panel_changed(bool state);

signals:
	void signal_start_toggled(bool state);
	void signal_cycle_clicked();
	void signal_clear_clicked();
	void signal_oprq_clicked();
	void signal_clock_toggled(bool state);
};

#endif // CONTROLPANEL_H

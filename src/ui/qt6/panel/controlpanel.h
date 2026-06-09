#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QMouseEvent>
#include "switch.h"
#include "led.h"
#include "buswleds.h"
#include "binarykeys.h"
#include "rotary.h"
#include "ignition.h"

struct sw_desc {
	QRect r;
	bool momentary;
	QString snd_on;
	QString snd_off;
};

#define LED_CNT 11
#define SW_CNT 12

class ControlPanel : public QWidget
{
	Q_OBJECT

private:
	QPixmap plane[16];
	int width, height;
	QRect crop;

	// individual status LEDs and control switches owned by the panel; not
	// exposed - callers drive them through the named slots/signals below. The
	// homogeneous 16-element rows live in their own widgets (keys, wleds).
	Switch *sw[SW_CNT];
	LED *led[LED_CNT];

	bool check_ignition(QPoint &m);
	void change_dimensions(const QRect &rect);

protected:
	void paintEvent(QPaintEvent *event);
	QSize sizeHint() const;
	QSize minimumSizeHint() const;

public:
	explicit ControlPanel(QWidget *parent = nullptr);
	QSizePolicy sizePolicy();

	BinaryKeys *keys;
	BusWLeds *wleds;
	Rotary *rotary;
	Ignition *ignition;

	void dim(bool state);
	void set_volume(int volume_percent);

public slots:
	void slot_state_changed(int state);
	void slot_small_panel_changed(bool state);
	void slot_set_alarm(bool state);
	void slot_set_p(bool state);
	void slot_set_clock(bool state);
	void slot_set_mode(bool state);
	void slot_set_stopn(bool state);

signals:
	void signal_start_toggled(bool state);
	void signal_clock_toggled(bool state);
	void signal_mode_toggled(bool state);
	void signal_cycle_clicked();
	void signal_clear_clicked();
	void signal_oprq_clicked();
	void signal_load_clicked();
	void signal_fetch_clicked();
	void signal_store_clicked();
	void signal_bin_clicked();
	void signal_step_clicked();
	void signal_stopn_clicked();
};

#endif // CONTROLPANEL_H

// vim: tabstop=4 shiftwidth=4 autoindent

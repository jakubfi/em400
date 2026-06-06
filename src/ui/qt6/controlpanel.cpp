#include <math.h>

#include <QDebug>
#include <QPainter>
#include <QLineF>
#include <QAudio>

#include "controlpanel.h"
#include "libem400.h"

static const int led_l_top = 253;
static const int led_width = 24;
static const int led_height = 24;

// Status LEDs only; the 16 W-bus LEDs of the upper row live in BusWLeds.
// Order matches enum sw_led_id (LED_MODE .. LED_ON).
static const QRect led_data[] = {
	QRect(167, led_l_top, led_width, led_height),  // LED_MODE
	QRect(203, led_l_top, led_width, led_height),  // LED_STOPN
	QRect(490, led_l_top, led_width, led_height),  // LED_CLOCK
	QRect(526, led_l_top, led_width, led_height),  // LED_Q
	QRect(561, led_l_top, led_width, led_height),  // LED_P
	QRect(597, led_l_top, led_width, led_height),  // LED_MC
	QRect(633, led_l_top, led_width, led_height),  // LED_IRQ
	QRect(1030, 123, led_width, led_height),        // LED_RUN
	QRect(1066, 123, led_width, led_height),        // LED_WAIT
	QRect(1102, 123, led_width, led_height),        // LED_ALARM
	QRect(1075, 195, led_width, led_height),        // LED_ON
};

static const int sw_l_top = 294;
static const int sw_width = 36;
static const int sw_height = 48;

#define SND_S "qrc:/sounds/switches/"

// Control switches only; the 16 binary data keys of the upper row live in
// BinaryKeys. Order matches enum sw_lower_id (SW_STEP .. SW_OPRQ).
static const struct sw_desc sw_data[] = {
	{ QRect(124, sw_l_top, sw_width, sw_height), true,  SND_S "step-1.wav",  SND_S "step-0.wav" },
	{ QRect(160, sw_l_top, sw_width, sw_height), false, SND_S "mode-1.wav",  SND_S "mode-0.wav" },
	{ QRect(196, sw_l_top, sw_width, sw_height), true,  SND_S "stopn-1.wav", SND_S "stopn-0.wav" },
	{ QRect(232, sw_l_top, sw_width, sw_height), true,  SND_S "cycle-1.wav", SND_S "cycle-0.wav" },
	{ QRect(268, sw_l_top, sw_width, sw_height), true,  SND_S "load-1.wav",  SND_S "load-0.wav" },
	{ QRect(304, sw_l_top, sw_width, sw_height), true,  SND_S "store-1.wav", SND_S "store-0.wav" },
	{ QRect(340, sw_l_top, sw_width, sw_height), true,  SND_S "fetch-1.wav", SND_S "fetch-0.wav" },
	{ QRect(376, sw_l_top, sw_width, sw_height), false, SND_S "start-1.wav", SND_S "start-0.wav" },
	{ QRect(411, sw_l_top, sw_width, sw_height), true,  SND_S "bin-1.wav",   SND_S "bin-0.wav" },
	{ QRect(447, sw_l_top, sw_width, sw_height), true,  SND_S "clear-1.wav", SND_S "clear-0.wav" },
	{ QRect(483, sw_l_top, sw_width, sw_height), false, SND_S "clock-1.wav", SND_S "clock-0.wav" },
	{ QRect(663, sw_l_top, sw_width, sw_height), true,  SND_S "oprq-1.wav",  SND_S "oprq-0.wav" }
};

static const QRect rotary_rect = QRect(746, 114, 232, 232);

#define SND_R "qrc:/sounds/rotary/"

static const QUrl rotary_sounds_r[] = {
	QUrl(SND_R "r00.wav"),
	QUrl(SND_R "r01.wav"),
	QUrl(SND_R "r02.wav"),
	QUrl(SND_R "r03.wav"),
	QUrl(SND_R "r04.wav"),
	QUrl(SND_R "r05.wav"),
	QUrl(SND_R "r06.wav"),
	QUrl(SND_R "r07.wav"),
	QUrl(SND_R "r08.wav"),
	QUrl(SND_R "r09.wav"),
	QUrl(SND_R "r10.wav"),
	QUrl(SND_R "r11.wav"),
	QUrl(SND_R "r12.wav"),
	QUrl(SND_R "r13.wav"),
	QUrl(SND_R "r14.wav"),
	QUrl(SND_R "r15.wav"),
};

static const QUrl rotary_sounds_l[] = {
	QUrl(SND_R "l00.wav"),
	QUrl(SND_R "l01.wav"),
	QUrl(SND_R "l02.wav"),
	QUrl(SND_R "l03.wav"),
	QUrl(SND_R "l04.wav"),
	QUrl(SND_R "l05.wav"),
	QUrl(SND_R "l06.wav"),
	QUrl(SND_R "l07.wav"),
	QUrl(SND_R "l08.wav"),
	QUrl(SND_R "l09.wav"),
	QUrl(SND_R "l10.wav"),
	QUrl(SND_R "l11.wav"),
	QUrl(SND_R "l12.wav"),
	QUrl(SND_R "l13.wav"),
	QUrl(SND_R "l14.wav"),
	QUrl(SND_R "l15.wav"),
};

static const QRect ignition_rect = QRect(1026, 234, 126, 126);

#define SND_I "qrc:/sounds/ignition/"

static const QUrl ignition_sounds_r[] = {
	QUrl(SND_I "off-l.wav"), // never happen
	QUrl(SND_I "on-r.wav"),
	QUrl(SND_I "lock-r.wav"),
};

static const QUrl ignition_sounds_l[] = {
	QUrl(SND_I "off-l.wav"),
	QUrl(SND_I "on-l.wav"),
	QUrl(SND_I "lock-r.wav"), // never happen
};

// -----------------------------------------------------------------------
ControlPanel::ControlPanel(QWidget *parent):
	QWidget(parent)
{
	for (int p=0 ; p<16 ; p++) {
		QString filename = QString("://pulpit/%1.png").arg(p, 2, 10, QLatin1Char('0'));
		plane[p].load(filename);
	}

	int i=0;
	for (auto const& sw_d : sw_data) {
		sw[i] = new Switch(plane[1].copy(sw_d.r), sw_d.snd_on, sw_d.snd_off, sw_d.momentary, this);
		i++;
	}

	i=0;
	for (auto const& led_d : led_data) {
		led[i] = new LED(plane[1].copy(led_d), this);
		i++;
	}

	keys = new BinaryKeys(plane[1], this);
	wleds = new BusWLeds(plane[1], this);

	QPixmap gfx[16];
	for (i=0 ; i<16 ; i++) {
		gfx[i] = plane[i].copy(rotary_rect);
	}
	rotary = new Rotary(gfx, rotary_sounds_r, rotary_sounds_l, this);

	for (i=0 ; i<3 ; i++) {
		gfx[i] = plane[i].copy(ignition_rect);
	}
	ignition = new Ignition(gfx, ignition_sounds_r, ignition_sounds_l, this);

	connect(ignition, &Ignition::signal_power, led[LED_ON], &LED::slot_change);

	change_dimensions(plane[0].rect());
	set_volume(100);
}

// -----------------------------------------------------------------------
void ControlPanel::set_volume(int volume_percent)
{
	qreal log_volume = volume_percent / (qreal) 100.0;
	qreal linear_volume = QAudio::convertVolume(log_volume, QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale);
	rotary->set_volume(linear_volume);
	ignition->set_volume(linear_volume);
	keys->set_volume(linear_volume);
	for (int i=0 ; i<SW_CNT ; i++) {
		sw[i]->set_volume(linear_volume);
	}
}

// -----------------------------------------------------------------------
void ControlPanel::change_dimensions(const QRect &rect)
{
	crop = rect;

	width = crop.width();
	height = crop.height();

	resize(width, height);
	QWidget::updateGeometry();

	for (int i=0 ; i<LED_CNT ; i++) {
		led[i]->move(led_data[i].topLeft() - crop.topLeft());
	}
	for (int i=0 ; i<SW_CNT ; i++) {
		sw[i]->move(sw_data[i].r.topLeft() - crop.topLeft());
	}
	keys->move(keys->origin() - crop.topLeft());
	wleds->move(wleds->origin() - crop.topLeft());
	rotary->move(rotary_rect.topLeft() - crop.topLeft());
	ignition->move(ignition_rect.topLeft() - crop.topLeft());
}

// -----------------------------------------------------------------------
void ControlPanel::paintEvent(QPaintEvent *event)
{
	QPainter painter;
	painter.begin(this);

	painter.drawPixmap(0, 0, plane[0].copy(crop));

	painter.end();
}

// -----------------------------------------------------------------------
QSize ControlPanel::sizeHint() const
{
	return QSize(width, height);
}

// -----------------------------------------------------------------------
QSize ControlPanel::minimumSizeHint() const
{
	return QSize(width, height);
}

// -----------------------------------------------------------------------
QSizePolicy ControlPanel::sizePolicy()
{
	return QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
}

// -----------------------------------------------------------------------
void ControlPanel::slot_state_changed(int state)
{
	switch (state) {
		case EM400_STATE_RUN:
			led[LED_RUN]->set(true);
			led[LED_WAIT]->set(false);
			dim(true);
			break;
		case EM400_STATE_WAIT:
			led[LED_RUN]->set(false);
			led[LED_WAIT]->set(true);
			dim(false);
			break;
		default:
			led[LED_RUN]->set(false);
			led[LED_WAIT]->set(false);
			dim(false);
			break;
	}
}

// -----------------------------------------------------------------------
void ControlPanel::slot_small_panel_changed(bool state)
{
	if (state) change_dimensions(plane[0].rect().adjusted(75, 72, -77, -66));
	else change_dimensions(plane[0].rect());
}

// -----------------------------------------------------------------------
void ControlPanel::dim(bool state)
{
	wleds->dim(state);
}

// vim: tabstop=4 shiftwidth=4 autoindent

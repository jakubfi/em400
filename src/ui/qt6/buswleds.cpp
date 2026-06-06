#include "buswleds.h"

// Geometry of the 16 W-bus LEDs on the upper panel row, in plane coordinates.
// They share a single row (top) and a fixed size; only the column varies.
static const int led_top = 122;
static const int led_width = 24;
static const int led_height = 24;
static const int led_x[16] = {
	131, 167, 203, 239, 275, 311, 347, 383,
	419, 455, 491, 527, 562, 598, 634, 670,
};

// -----------------------------------------------------------------------
BusWLeds::BusWLeds(const QPixmap &plane, QWidget *parent)
	: QWidget(parent)
{
	m_origin = QPoint(led_x[0], led_top);
	resize((led_x[15] + led_width) - led_x[0], led_height);

	for (int i=0 ; i<16 ; i++) {
		QRect r(led_x[i], led_top, led_width, led_height);
		led[i] = new LED(plane.copy(r), this);
		led[i]->move(led_x[i] - m_origin.x(), 0);
	}
}

// -----------------------------------------------------------------------
void BusWLeds::slot_set_value(uint16_t val)
{
	for (int i=0 ; i<16 ; i++) {
		led[i]->set(val & (1 << (15-i)));
	}
}

// -----------------------------------------------------------------------
void BusWLeds::dim(bool state)
{
	for (int i=0 ; i<16 ; i++) {
		led[i]->dim(state);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent

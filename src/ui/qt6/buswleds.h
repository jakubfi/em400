#ifndef BUSWLEDS_H
#define BUSWLEDS_H

#include <QWidget>
#include <QPoint>
#include "led.h"

// The 16 LEDs of the upper panel row, showing the value present on the system
// bus "W". A transparent overlay that owns its LEDs and the bit-to-LED mapping;
// the parent only needs to place it (see origin()) and feed it a 16-bit value.
class BusWLeds : public QWidget
{
	Q_OBJECT

public:
	explicit BusWLeds(const QPixmap &plane, QWidget *parent = nullptr);

	// Top-left of the LED row in panel (plane) coordinates, so the parent can
	// position the overlay accounting for its own crop offset.
	QPoint origin() const { return m_origin; }

	// Dim the row (used to fade the bus display while the CPU is running).
	void dim(bool state);

public slots:
	void slot_set_value(uint16_t val);

private:
	LED *led[16];
	QPoint m_origin;
};

#endif // BUSWLEDS_H

// vim: tabstop=4 shiftwidth=4 autoindent

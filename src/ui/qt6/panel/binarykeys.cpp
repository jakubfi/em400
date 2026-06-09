#include <QPainter>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QString>

#include "binarykeys.h"

// Geometry of the 16 binary data keys on the upper panel row, in plane
// coordinates. They share a single row (top) and a fixed size; only the column
// varies.
static const int sw_top = 163;
static const int sw_width = 36;
static const int sw_height = 48;
static const int sw_x[16] = {
	125, 161, 196, 232, 268, 304, 340, 376,
	412, 447, 483, 519, 555, 591, 627, 663,
};

#define SND_S "qrc:/sounds/switches/"

// -----------------------------------------------------------------------
BinaryKeys::BinaryKeys(const QPixmap &plane, QWidget *parent)
	: QWidget(parent)
{
	m_origin = QPoint(sw_x[0], sw_top);
	resize((sw_x[15] + sw_width) - sw_x[0], sw_height);

	for (int i=0 ; i<16 ; i++) {
		QRect r(sw_x[i], sw_top, sw_width, sw_height);
		key[i].on = plane.copy(r);
		key[i].snd_on.setSource(QUrl(QString(SND_S "%1-1.wav").arg(i, 2, 10, QLatin1Char('0'))));
		key[i].snd_off.setSource(QUrl(QString(SND_S "%1-0.wav").arg(i, 2, 10, QLatin1Char('0'))));
	}
}

// -----------------------------------------------------------------------
uint16_t BinaryKeys::value() const
{
	uint16_t v = 0;
	for (int i=0 ; i<16 ; i++) {
		v |= (uint16_t) key[i].state << (15-i);
	}
	return v;
}

// -----------------------------------------------------------------------
void BinaryKeys::set_volume(qreal linear_volume)
{
	for (int i=0 ; i<16 ; i++) {
		key[i].snd_on.setVolume(linear_volume);
		key[i].snd_off.setVolume(linear_volume);
	}
}

// -----------------------------------------------------------------------
// Returns the index of the key covering local x, or -1 if x falls in a gap.
int BinaryKeys::key_at(int x) const
{
	for (int i=0 ; i<16 ; i++) {
		int left = sw_x[i] - m_origin.x();
		if (x >= left && x < left + sw_width) return i;
	}
	return -1;
}

// -----------------------------------------------------------------------
void BinaryKeys::set_key(int i, bool state)
{
	if (key[i].state == state) return;

	key[i].state = state;
	if (state) {
		key[i].snd_on.play();
	} else {
		key[i].snd_off.play();
	}

	update();
	emit signal_value_changed(value());
}

// -----------------------------------------------------------------------
void BinaryKeys::paintEvent(QPaintEvent *event)
{
	QPainter painter;
	painter.begin(this);

	for (int i=0 ; i<16 ; i++) {
		if (key[i].state) painter.drawPixmap(sw_x[i] - m_origin.x(), 0, key[i].on);
	}

	painter.end();
}

// -----------------------------------------------------------------------
// Press toggles the hit key and arms a sweep: dragging the held button across
// the row paints that same new state onto every key the cursor passes over.
void BinaryKeys::mousePressEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton) return;

	int i = key_at((int) event->position().x());
	if (i < 0) return;

	sweep_target = !key[i].state;
	sweeping = true;
	set_key(i, sweep_target);
}

// -----------------------------------------------------------------------
void BinaryKeys::mouseMoveEvent(QMouseEvent *event)
{
	if (!sweeping) return;

	int i = key_at((int) event->position().x());
	if (i < 0) return;

	set_key(i, sweep_target);
}

// -----------------------------------------------------------------------
void BinaryKeys::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton) return;

	sweeping = false;
}

// -----------------------------------------------------------------------
void BinaryKeys::enterEvent(QEnterEvent *event)
{
	QGuiApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
}

// -----------------------------------------------------------------------
void BinaryKeys::leaveEvent(QEvent *event)
{
	QGuiApplication::restoreOverrideCursor();
}

// vim: tabstop=4 shiftwidth=4 autoindent

#include <QPainter>
#include <QGuiApplication>
#include <QDebug>
#include <math.h>
#include "ignition.h"

#define DEBUG_UI 0

// -----------------------------------------------------------------------
Ignition::Ignition(QPixmap gfx[3], const QUrl snd_rs[3], const QUrl snd_ls[3], QWidget *parent)
	: QWidget{parent}
{
	for (int i=0 ; i<3 ; i++) {
		this->gfx[i] = gfx[i];
		snd_r[i].setSource(snd_rs[i]);
		snd_l[i].setSource(snd_ls[i]);
	}

	resize(gfx[0].width(), gfx[0].height());
	center = gfx[0].rect().center();
	radius_outer = gfx[0].rect().width() / 2;
	radius_main = radius_outer - 24;
	radius_inner = radius_outer - 50;

	power_on_timer.setInterval(280);
	power_on_timer.setSingleShot(true);
	connect(&power_on_timer, &QTimer::timeout, this, &Ignition::power_on);

	setMouseTracking(true);
}

// -----------------------------------------------------------------------
void Ignition::set_volume(qreal linear_volume)
{
	for (int i=0 ; i<3 ; i++) {
		snd_r[i].setVolume(linear_volume);
		snd_l[i].setVolume(linear_volume);
	}
}

// -----------------------------------------------------------------------
void Ignition::power_on()
{
	emit signal_power(true);
}

// -----------------------------------------------------------------------
void Ignition::paintEvent(QPaintEvent *event)
{
	QPainter painter;
	painter.begin(this);

	painter.drawPixmap(0, 0, gfx[position]);

	if (DEBUG_UI) {
		QPen pen = QPen(Qt::DotLine);
		pen.setColor(QColor(0, 255, 0));
		painter.setPen(pen);

		painter.drawRect(gfx[0].rect());
		painter.drawEllipse(center, radius_outer, radius_outer);
		painter.drawEllipse(center, radius_main, radius_main);
		painter.drawEllipse(center, radius_inner, radius_inner);
		painter.drawLine(gfx[0].rect().left(), center.y(), gfx[0].rect().right(), center.y());
		painter.drawLine(center.x(), gfx[0].rect().top(), center.x(), gfx[0].rect().bottom());
	}

	painter.end();
}

// --------------------------------------------------------------------------
int Ignition::pos_from_point(QPoint &m)
{
	// calculate angular "m" position
	double mdy = (center.y() - m.y());
	double mdx = (center.x() - m.x());
	double angle = atan2(mdy, mdx);
	// make it 0-2pi
	if (angle < 0) angle += 2*M_PI;

	// find which slice "m" is in
	double slice = (2*M_PI) / 4;
	int pos;
	for (pos=0 ; pos<4 ; pos++) {
		double s = (double) pos * slice;
		if ((angle > s-slice/2) && (angle <= s+slice/2)) break;
	}
	return (pos+1) % 4;
}

// -----------------------------------------------------------------------
void Ignition::mousePressEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton) return;

	QPoint m = event->pos();
	if (can_interact_outer) {
		int new_pos = pos_from_point(m);
		// only 3 positions for ignition
		if (new_pos == 3) new_pos = 2;
		if (abs(new_pos-position) == 1) {
			if (new_pos > position) snd_r[new_pos].play();
			else snd_l[new_pos].play();
			if ((position == 0) && (new_pos == 1)) power_on_timer.start();
			if (new_pos == 0) emit signal_power(false);
			if (new_pos == 2) emit signal_locked(true);
			if ((position == 2) && (new_pos == 1)) emit signal_locked(false);
			position = new_pos;
			update();
		}
	} else if (can_interact_inner) {
		dragging = true;
	}
}

// -----------------------------------------------------------------------
void Ignition::mouseReleaseEvent(QMouseEvent *event)
{
	dragging = false;
}

// -----------------------------------------------------------------------
void Ignition::mouseMoveEvent(QMouseEvent *event)
{
	if (dragging) {
		QPoint m = event->pos();
		if (sqrt(pow(center.x()-m.x(), 2) + pow(center.y()-m.y(), 2)) >= radius_inner) {
			int new_pos = pos_from_point(m);
			if (new_pos == 3) new_pos = 2;
			if (abs(new_pos-position) == 1) {
				if (new_pos > position) snd_r[new_pos].play();
				else snd_l[new_pos].play();
				if ((position == 0) && (new_pos == 1)) power_on_timer.start();
				if (new_pos == 0) emit signal_power(false);
				if (new_pos == 2) emit signal_locked(true);
				if ((position == 2) && (new_pos == 1)) emit signal_locked(false);
				position = new_pos;
				update();
			}
		}
	} else {
		QPoint m = event->pos();
		bool was_can_interact = can_interact_inner || can_interact_outer;
		can_interact_outer = false;
		can_interact_inner = false;
		if (sqrt(pow(center.x()-m.x(), 2) + pow(center.y()-m.y(), 2)) <= radius_outer) {
			// within the actual widget area
			if (sqrt(pow(center.x()-m.x(), 2) + pow(center.y()-m.y(), 2)) >= radius_main) {
				// outer ring
				can_interact_outer = true;
			} else if (sqrt(pow(center.x()-m.x(), 2) + pow(center.y()-m.y(), 2)) >= radius_inner) {
				// inner ring
				if (pos_from_point(m) == position) {
					// just the tip
					can_interact_inner = true;
				}
			}
		}

		if (was_can_interact && !can_interact_inner && ! can_interact_outer) {
			QGuiApplication::restoreOverrideCursor();
		}
		if (!was_can_interact && (can_interact_inner || can_interact_outer)) {
			QGuiApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
		}
	}
}
// -----------------------------------------------------------------------
void Ignition::leaveEvent(QEvent *event)
{
	can_interact_inner = false;
	can_interact_outer = false;
	QGuiApplication::restoreOverrideCursor();
}

#include <QPainter>
#include <QGuiApplication>
#include <QDebug>
#include <math.h>
#include "rotary.h"

#define DEBUG_UI 0

// -----------------------------------------------------------------------
Rotary::Rotary(QPixmap gfx[16], const QUrl snd_rs[16], const QUrl snd_ls[16], QWidget *parent)
	: QWidget{parent}
{
	int i;
	for (i=0 ; i<16 ; i++) {
		this->gfx[i] = gfx[i];
		snd_r[i].setSource(snd_rs[i]);
		snd_l[i].setSource(snd_ls[i]);
	}
	this->resize(gfx[0].width(), gfx[0].height());
	center = gfx[0].rect().center();
	radius_outer = gfx[0].rect().width() / 2;
	radius_main = radius_outer - 50;
	radius_inner = radius_main - 40;
	connect(&anim_timer, &QTimer::timeout, this, &Rotary::anim_step);
	setMouseTracking(true);
}

// -----------------------------------------------------------------------
void Rotary::paintEvent(QPaintEvent *event)
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
		for (int i=0 ; i<16 ; i++) {
			QLineF angleline;
			angleline.setP1(center);
			angleline.setAngle(i*360/16);
			angleline.setLength(radius_outer);
			painter.drawLine(angleline);
		}
	}

	painter.end();
}

// --------------------------------------------------------------------------
int Rotary::pos_from_point(QPoint &m)
{
	// calculate angular "m" position
	double mdy = (center.y() - m.y());
	double mdx = (center.x() - m.x());
	double angle = atan2(mdy, mdx);
	// make it 0-2pi
	if (angle < 0) angle += 2*M_PI;

	// find which slice "m" is in
	double slice = (2*M_PI) / 16;
	int pos;
	for (pos=0 ; pos<16 ; pos++) {
		double s = (double) pos * slice;
		if ((angle > s-slice/2) && (angle <= s+slice/2)) break;
	}
	return (pos+3) % 16;
}

// -----------------------------------------------------------------------
void Rotary::anim_step()
{
	if (anim_delta > 0) {
		anim_delta--;
		position = (position+1) % 16;
		snd_r[position].play();
	} else {
		anim_delta++;
		position--;
		if (position < 0) position += 16;
		snd_l[position].play();
	}
	anim_timer.setInterval(anim_timer.interval()-4);
	emit signal_rotated(position);
	update();

	if (anim_delta == 0) {
		anim_timer.stop();
	}
}

// -----------------------------------------------------------------------
void Rotary::mousePressEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton) return;

	QPoint m = event->pos();
	if (can_interact_outer) {
		int new_pos = pos_from_point(m);
		if (new_pos == position) return;
		anim_delta = new_pos - position;
		if (anim_delta > 8) {
			anim_delta = -(16 - anim_delta);
		}
		if (anim_delta < -8) {
			anim_delta = 16 + anim_delta;
		}
		anim_timer.setInterval(50);
		anim_timer.start();
	} else if (can_interact_inner) {
		dragging = true;
	}
}

// -----------------------------------------------------------------------
void Rotary::mouseReleaseEvent(QMouseEvent *event)
{
	dragging = false;
}

// -----------------------------------------------------------------------
void Rotary::mouseMoveEvent(QMouseEvent *event)
{
	if (dragging) {
		QPoint m = event->pos();
		if (sqrt(pow(center.x()-m.x(), 2) + pow(center.y()-m.y(), 2)) >= radius_inner) {
			int new_pos = pos_from_point(m);
			if (new_pos != position) {
				if (new_pos > position) snd_r[new_pos].play();
				else snd_l[new_pos].play();
				position = new_pos;
				emit signal_rotated(position);
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

// --------------------------------------------------------------------------
void Rotary::wheelEvent(QWheelEvent *event)
{
	QPointF m = event->position();
	if (sqrt(pow(center.x()-m.x(), 2) + pow(center.y()-m.y(), 2)) <= radius_main) {
		if (event->angleDelta().ry() < 0) {
			position -= 1;
			if (position < 0) position = 15;
			snd_l[position].play();
		} else {
			position = (position + 1) % 16;
			snd_r[position].play();
		}
		emit signal_rotated(position);
		update();
	}
}

// -----------------------------------------------------------------------
void Rotary::leaveEvent(QEvent *event)
{
	can_interact_inner = false;
	can_interact_outer = false;
	QGuiApplication::restoreOverrideCursor();
}

// -----------------------------------------------------------------------
void Rotary::set_position(int pos)
{
	position = pos;
	emit signal_rotated(position);
	update();
}

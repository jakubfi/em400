#include <QPainter>
#include <QDebug>
#include <math.h>
#include "ignition.h"

#define DEBUG_UI 0

// -----------------------------------------------------------------------
Ignition::Ignition(QPixmap gfx[3], const QUrl snd_rs[3], const QUrl snd_ls[3], QWidget *parent)
	: QWidget{parent}
{
	int i;
	for (i=0 ; i<3 ; i++) {
		this->gfx[i] = gfx[i];
		snd_r[i].setSource(snd_rs[i]);
		//snd_r[i].setVolume(0.7);
		snd_l[i].setSource(snd_ls[i]);
		//snd_l[i].setVolume(0.7);
	}
	this->resize(gfx[0].width(), gfx[0].height());
	center = gfx[0].rect().center();
	radius = gfx[0].rect().width() / 2;
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
		painter.drawEllipse(center, radius, radius);
		painter.drawLine(gfx[0].rect().left(), center.y(), gfx[0].rect().right(), center.y());
		painter.drawLine(center.x(), gfx[0].rect().top(), center.x(), gfx[0].rect().bottom());
	}

	painter.end();
}

// --------------------------------------------------------------------------
int Ignition::calculate_pos(QPoint &m)
{
	double mdy = (center.y() - m.y());
	double mdx = (center.x() - m.x());
	double result = atan2(mdy, mdx);
	if (result < 0) result += 2*M_PI;
	double slice = (2*M_PI) / 4;
	double hslice = slice / 2;
	int i;
	for (i=0 ; i<4 ; i++) {
		double s = (double)i * slice;
		if (s < 0) s += 2*M_PI;
		if ((result > s-hslice) && (result < s+hslice)) break;
	}
	i = (i+1) % 4;
	if (i > 2) i = position; // last position unavailable
	return i;
}

// -----------------------------------------------------------------------
void Ignition::mousePressEvent(QMouseEvent *event)
{
	QPoint m = event->pos();
	if (sqrt(pow(center.x()-m.x(), 2) + pow(center.y()-m.y(), 2)) <= radius) {
		dragging = true;
		int new_pos = calculate_pos(m);
		if (new_pos != position) {
			if (new_pos > position) snd_r[new_pos].play();
			else snd_l[new_pos].play();
			position = new_pos;
			update();
		}
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
	QPoint m = event->pos();
	if (sqrt(pow(center.x()-m.x(), 2) + pow(center.y()-m.y(), 2)) <= radius) {
		dragging = true;
		int new_pos = calculate_pos(m);
		if (new_pos != position) {
			if (new_pos > position) snd_r[new_pos].play();
			else snd_l[new_pos].play();
			position = new_pos;
			update();
		}
	}
}

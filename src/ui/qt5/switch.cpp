#include <QPainter>
#include <QGuiApplication>
#include <QMouseEvent>
#include "switch.h"

#define DEBUG_UI 0

// -----------------------------------------------------------------------
Switch::Switch(QPixmap gfx_on, QString snd_on, QString snd_off, bool momentary, QWidget *parent)
	: QWidget{parent}
	, momentary(momentary)
	, gfx_on(gfx_on)
{
	this->snd_on.setSource(QUrl(snd_on));
	this->snd_off.setSource(QUrl(snd_off));

	resize(gfx_on.width(), gfx_on.height());
}

// -----------------------------------------------------------------------
void Switch::set_volume(qreal linear_volume)
{
	snd_on.setVolume(linear_volume);
	snd_off.setVolume(linear_volume);
}

// -----------------------------------------------------------------------
void Switch::paintEvent(QPaintEvent *event)
{
	QPainter painter;
	painter.begin(this);

	if (state) painter.drawPixmap(0, 0, gfx_on);

	if (DEBUG_UI) {
		QPen pen = QPen(Qt::DotLine);
		pen.setColor(QColor(0, 255, 0));
		painter.setPen(pen);
		painter.drawRect(gfx_on.rect());
	}

	painter.end();
}

// -----------------------------------------------------------------------
void Switch::mousePressEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton) return;

	state = !state;
	if (state) {
		snd_on.play();
	} else {
		snd_off.play();
	}

	if (momentary && state) emit signal_clicked();
	else emit signal_toggled(state);

	update();
}

// -----------------------------------------------------------------------
void Switch::enterEvent(QEvent *event)
{
	QGuiApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
}

// -----------------------------------------------------------------------
void Switch::leaveEvent(QEvent *event)
{
	QGuiApplication::restoreOverrideCursor();
}

// -----------------------------------------------------------------------
void Switch::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton) return;

	if (momentary) {
		state = false;
		snd_off.play();
		update();
	}
}

// -----------------------------------------------------------------------
void Switch::set(bool state)
{
	this->state = state;
	update();
}

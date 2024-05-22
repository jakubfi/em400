#include <QPainter>
#include <QDebug>
#include "led.h"

#define DEBUG_UI 0

// -----------------------------------------------------------------------
LED::LED(QPixmap gfx_on, QWidget *parent)
	: QWidget{parent}
	, gfx_on(gfx_on)
{
	this->resize(gfx_on.width()+1, gfx_on.height()+1);
}

// -----------------------------------------------------------------------
void LED::paintEvent(QPaintEvent *event)
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
void LED::slot_change(bool state)
{
	this->state = state;
	update();
}

// -----------------------------------------------------------------------
void LED::set(bool state)
{
	this->state = state;
	update();
}

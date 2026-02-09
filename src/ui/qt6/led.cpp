#include <QPainter>
#include <QDebug>
#include "led.h"

#define DEBUG_UI 0

#define DIM_ON 0.7f
#define DIM_OFF 0.2f

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

	if (state) {
		painter.setOpacity(dim_on);
	} else {
		painter.setOpacity(dim_off);
	}
	painter.drawPixmap(0, 0, gfx_on);
	painter.setOpacity(1);

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

// -----------------------------------------------------------------------
void LED::dim(bool state)
{
	if (state) {
		dim_on = DIM_ON;
		dim_off = DIM_OFF;
	} else {
		dim_on = 1.0f;
		dim_off = 0.0f;
	}
}

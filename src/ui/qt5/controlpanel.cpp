#include <QPainter>
#include "controlpanel.h"

// -----------------------------------------------------------------------
ControlPanel::ControlPanel(QWidget *parent) :
	QWidget(parent)
{

}

// -----------------------------------------------------------------------
ControlPanel::~ControlPanel()
{

}

// -----------------------------------------------------------------------
void ControlPanel::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
}

// -----------------------------------------------------------------------
QSize ControlPanel::minimumSizeHint() const
{
	return QSize(0, 0);
}

// -----------------------------------------------------------------------
QSize ControlPanel::sizeHint() const
{
	return minimumSizeHint();
}

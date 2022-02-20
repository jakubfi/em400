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
	auto image = QImage("/home/amo/dt-exported/DSC09258.JPG_uszy.jpg");
	painter.drawImage(0, 0,	image.scaled(width(), height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

// -----------------------------------------------------------------------
QSize ControlPanel::minimumSizeHint() const
{
	return QSize(1000, 400);
}

// -----------------------------------------------------------------------
QSize ControlPanel::sizeHint() const
{
	return minimumSizeHint();
}

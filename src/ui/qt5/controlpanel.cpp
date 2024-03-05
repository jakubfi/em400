#include <QDebug>
#include <QPainter>
#include "controlpanel.h"

// -----------------------------------------------------------------------
ControlPanel::ControlPanel(QWidget *parent) :
    QWidget(parent)
{
    bg.load("://pulpit/0.jpg");
    fg.load("://pulpit/1.jpg");

    this->scale = (double) bg.width() / (double) this->bg_width;

    const int crop = 264;
    const int offset_top = 10;
    QRect crop_rect(crop, crop + offset_top, bg.width() - 2*crop, bg.height() - 2*crop);

    bg = bg.copy(crop_rect).scaledToWidth(this->bg_width, Qt::SmoothTransformation);
    fg = fg.copy(crop_rect).scaledToWidth(this->bg_width, Qt::SmoothTransformation);

    this->bg_height = bg.height();
    this->which = 0;
}

// -----------------------------------------------------------------------
ControlPanel::~ControlPanel()
{

}

// -----------------------------------------------------------------------
void ControlPanel::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);

    painter.drawPixmap(0, 0, bg);
    if (which) {
        const int led_bar_x = (float) (511-264) / this->scale;
        const int led_bar_y = (float) (476-264-10) / this->scale;
        const int led_bar_wh = (float) 82 / this->scale;
        qDebug() << led_bar_x * this->scale+264 << led_bar_y * this->scale+264;
        QRect crop_rect(led_bar_x, led_bar_y, led_bar_wh, led_bar_wh);
        QPixmap led = fg.copy(crop_rect);
        painter.drawPixmap(led_bar_x, led_bar_y, led);
        QPen pen(Qt::green, 1);
        painter.setPen(pen);
        painter.drawRect(crop_rect);
    }

    painter.end();
}

// -----------------------------------------------------------------------
QSize ControlPanel::sizeHint() const
{
    return QSize(this->bg_width, this->bg_height);
}

// -----------------------------------------------------------------------
QSizePolicy ControlPanel::sizePolicy()
{
    return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

// --------------------------------------------------------------------------
void ControlPanel::mousePressEvent(QMouseEvent *event)
{
    this->which = 1;
    update();
}

// --------------------------------------------------------------------------
void ControlPanel::mouseReleaseEvent(QMouseEvent *event)
{
    this->which = 0;
    update();
}

#include <QPainter>
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
        //snd_r[i].setVolume(0.7);
        snd_l[i].setSource(snd_ls[i]);
        //snd_l[i].setVolume(0.7);
    }
    this->resize(gfx[0].width(), gfx[0].height());
    center = gfx[0].rect().center();
    radius = gfx[0].rect().width() / 2;
    radius_inner = radius - 50;
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
        painter.drawEllipse(center, radius, radius);
        painter.drawEllipse(center, radius_inner, radius_inner);
        for (int i=0 ; i<16 ; i++) {
            QLineF angleline;
            angleline.setP1(center);
            angleline.setAngle(i*360/16);
            angleline.setLength(radius);
            painter.drawLine(angleline);
        }

    }

    painter.end();
}

// --------------------------------------------------------------------------
int Rotary::calculate_pos(QPoint &m)
{
    double mdy = (center.y() - m.y());
    double mdx = (center.x() - m.x());
    double result = atan2(mdy, mdx);
    if (result < 0) result += 2*M_PI;
    double slice = (2*M_PI) / 16;
    double hslice = slice / 2;
    int i;
    for (i=0 ; i<16 ; i++) {
        double s = (double)i * slice;
        if (s < 0) s += 2*M_PI;
        if ((result > s-hslice) && (result < s+hslice)) break;
    }
    return (i+3) % 16;
}

// -----------------------------------------------------------------------
void Rotary::mousePressEvent(QMouseEvent *event)
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
void Rotary::mouseReleaseEvent(QMouseEvent *event)
{
    dragging = false;
}

// -----------------------------------------------------------------------
void Rotary::mouseMoveEvent(QMouseEvent *event)
{
    if (dragging) {
        QPoint m = event->pos();
        int new_pos = calculate_pos(m);
        if (new_pos != position) {
            if (new_pos > position) snd_r[new_pos].play();
            else snd_l[new_pos].play();
            position = new_pos;
            update();
        }
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

// --------------------------------------------------------------------------
void Rotary::wheelEvent(QWheelEvent *event)
{
    QPointF m = event->position();
    if (sqrt(pow(center.x()-m.x(), 2) + pow(center.y()-m.y(), 2)) <= radius_inner) {
        if (event->angleDelta().ry() < 0) {
            position -= 1;
            if (position < 0) position = 15;
            snd_l[position].play();
        } else {
            position = (position + 1) % 16;
            snd_r[position].play();
        }
        update();
    }
}

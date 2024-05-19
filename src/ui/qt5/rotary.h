#ifndef ROTARY_H
#define ROTARY_H

#include <QWidget>
#include <QMouseEvent>
#include <QtMultimedia/QSoundEffect>

class Rotary : public QWidget
{
    Q_OBJECT

private:
    int position = 8;
    QPixmap gfx[16];
    QSoundEffect snd_r[16], snd_l[16];
    QPoint center;
    int radius, radius_inner;
    bool dragging;

    int calculate_pos(QPoint &m);

public:
    explicit Rotary(QPixmap gfx[16], const QUrl snd_rs[16], const QUrl snd_ls[16], QWidget *parent);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

signals:
    void signal_rotated(int position);

};

#endif // ROTARY_H

#ifndef ROTARY_H
#define ROTARY_H

#include <QWidget>
#include <QMouseEvent>
#include <QTimer>
#include <QtMultimedia/QSoundEffect>

class Rotary : public QWidget
{
    Q_OBJECT

private:
    int position = 8;
    QPixmap gfx[16];
    QSoundEffect snd_r[16], snd_l[16];
    QPoint center;
	int radius_outer, radius_main, radius_inner;
    bool dragging;
	int anim_delta = 0;
	QTimer anim_timer;

    int pos_from_point(QPoint &m);
	void anim_step();

public:
    explicit Rotary(QPixmap gfx[16], const QUrl snd_rs[16], const QUrl snd_ls[16], QWidget *parent);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);

signals:
    void signal_rotated(int position);

};

#endif // ROTARY_H

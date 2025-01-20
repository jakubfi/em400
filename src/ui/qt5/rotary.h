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
	bool dragging = false;
	int anim_delta = 0;
	QTimer anim_timer;
	bool can_interact_outer = false;
	bool can_interact_inner = false;

	int pos_from_point(QPoint &m);
	void anim_step();

public:
	explicit Rotary(QPixmap gfx[16], const QUrl snd_rs[16], const QUrl snd_ls[16], QWidget *parent);
	void set_position(int pos);
	void set_volume(qreal linear_volume);

protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
	void leaveEvent(QEvent *event);

signals:
	void signal_rotated(int position);

};

#endif // ROTARY_H

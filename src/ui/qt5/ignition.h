#ifndef IGNITION_H
#define IGNITION_H

#include <QWidget>
#include <QMouseEvent>
#include <QtMultimedia/QSoundEffect>

class Ignition : public QWidget
{
	Q_OBJECT

private:
	int position = 0;
	QPixmap gfx[3];
	QSoundEffect snd_r[3], snd_l[3];
	QPoint center;
	int radius_main, radius_outer, radius_inner;
	bool dragging = false;
	bool can_interact_outer = false;
	bool can_interact_inner = false;

	int pos_from_point(QPoint &m);

public:
	explicit Ignition(QPixmap gfx[3], const QUrl snd_rs[3], const QUrl snd_ls[3], QWidget *parent = nullptr);

protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void leaveEvent(QEvent *event);

signals:
	void signal_rotated(int position);
};

#endif // IGNITION_H

#ifndef IGNITION_H
#define IGNITION_H

#include <QWidget>
#include <QTimer>
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
	QTimer power_on_timer;
	QTimer step_timer;
	int pending_pos = 0;
	int radius_main, radius_outer, radius_inner;
	bool dragging = false;
	bool can_interact_outer = false;
	bool can_interact_inner = false;

	int pos_from_point(QPoint &m);
	void step(int new_pos);
	void move_to(int new_pos);

public:
	explicit Ignition(QPixmap gfx[3], const QUrl snd_rs[3], const QUrl snd_ls[3], QWidget *parent = nullptr);
	void set_volume(qreal linear_volume);
	void set_position(int pos);

protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void leaveEvent(QEvent *event);

signals:
	void signal_power(bool state);
	void signal_locked(bool state);

private slots:
	void power_on();
};

#endif // IGNITION_H

// vim: tabstop=4 shiftwidth=4 autoindent

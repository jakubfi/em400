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
	int radius;
	bool dragging;

	int calculate_pos(QPoint &m);

public:
	explicit Ignition(QPixmap gfx[3], const QUrl snd_rs[3], const QUrl snd_ls[3], QWidget *parent = nullptr);

protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

signals:

};

#endif // IGNITION_H

#ifndef SWITCH_H
#define SWITCH_H

#include <QWidget>
#include <QtMultimedia/QSoundEffect>

class Switch : public QWidget
{
	Q_OBJECT

private:
	bool momentary;
	QPixmap gfx_on;
	QSoundEffect snd_on, snd_off;
	bool state = false;

public:
	explicit Switch(QPixmap gfx_on, QString snd_on, QString snd_off, bool momentary = false, QWidget *parent = nullptr);
	bool get() { return state; }
	void set(bool state);
	void set_volume(qreal linear_volume);

protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);

signals:
	void signal_toggled(bool state);
	void signal_clicked();
};

#endif // SWITCH_H

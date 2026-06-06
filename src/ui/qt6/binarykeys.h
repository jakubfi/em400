#ifndef BINARYKEYS_H
#define BINARYKEYS_H

#include <QWidget>
#include <QMouseEvent>
#include <QSoundEffect>
#include <QPixmap>

class BinaryKeys : public QWidget
{
	Q_OBJECT

public:
	explicit BinaryKeys(const QPixmap &plane, QWidget *parent = nullptr);
	QPoint origin() const { return m_origin; }

	uint16_t value() const;
	void set_volume(qreal linear_volume);

signals:
	void signal_value_changed(uint16_t val);

protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void enterEvent(QEnterEvent *event);
	void leaveEvent(QEvent *event);

private:
	struct Key {
		QPixmap on;
		QSoundEffect snd_on;
		QSoundEffect snd_off;
		bool state = false;
	};

	int key_at(int x) const;
	void set_key(int i, bool state);

	Key key[16];
	QPoint m_origin;
	bool sweeping = false;
	bool sweep_target = false;
};

#endif // BINARYKEYS_H

// vim: tabstop=4 shiftwidth=4 autoindent

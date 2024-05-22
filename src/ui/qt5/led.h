#ifndef LED_H
#define LED_H

#include <QWidget>

class LED : public QWidget
{
	Q_OBJECT

private:
	QPixmap gfx_on;
	bool state = false;

protected:
	void paintEvent(QPaintEvent *event);

public:
	explicit LED(QPixmap gfx_on, QWidget *parent = nullptr);
	void set(bool state);

public slots:
    void slot_change(bool state);
};

#endif // LED_H

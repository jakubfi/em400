#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>


class ControlPanel : public QWidget
{
	Q_OBJECT

public:
	explicit ControlPanel(QWidget *parent = nullptr);
	~ControlPanel();

private:

protected:
	void paintEvent(QPaintEvent *event);
	QSize minimumSizeHint() const;
	QSize sizeHint() const;
};

#endif // CONTROLPANEL_H

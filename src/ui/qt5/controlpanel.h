#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QMouseEvent>


class ControlPanel : public QWidget
{
	Q_OBJECT

private:
    float scale;
    int bg_width = 1100;
    int bg_height;
    QPixmap bg;
    QPixmap fg;
    int which = 0;

protected:
	void paintEvent(QPaintEvent *event);
    QSize sizeHint() const;
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

public:
    explicit ControlPanel(QWidget *parent = nullptr);
    ~ControlPanel();
    QSizePolicy sizePolicy();

};

#endif // CONTROLPANEL_H

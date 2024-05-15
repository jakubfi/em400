#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QMouseEvent>


class ControlPanel : public QWidget
{
	Q_OBJECT

private:
    QPixmap bg;
    QPixmap fg;
    int width, height;
    uint16_t w;
    int state;
    bool q, p, mc, irq;

    void paint_bus_w(QPainter &painter);
    void paint_state(QPainter &painter);
    void paint_q(QPainter &painter);

protected:
	void paintEvent(QPaintEvent *event);
    QSize sizeHint() const;
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

public:
    explicit ControlPanel(QWidget *parent = nullptr);
    ~ControlPanel();
    QSizePolicy sizePolicy();

public slots:
    void cpu_bus_w_changed(uint16_t val);
    void cpu_state_changed(int state);
    void cpu_reg_changed(int reg, uint16_t val);
};

#endif // CONTROLPANEL_H

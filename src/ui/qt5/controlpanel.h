#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QMouseEvent>

struct sw_desc {
    QRect r;
    bool momentary;
};

class ControlPanel : public QWidget
{
	Q_OBJECT

private:
    QPixmap plane[16];
    int width, height;
    uint16_t w;
    int state;
    bool q, p, mc, irq;
    bool sw_u_state[16] = {false};
    bool sw_l_state[12] = {false};
    bool *curr_sw = NULL;

    void paint_bus_w(QPainter &painter);
    void paint_state(QPainter &painter);
    void paint_q(QPainter &painter);
    void paint_sw_u(QPainter &painter);
    void paint_sw_l(QPainter &painter);

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

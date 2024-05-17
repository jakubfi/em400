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
    bool q, p, alarm, clock, irq;
    int rotary = 8;
    bool dragging_rotary = false;
    bool sw_u_state[16] = {false};
    bool sw_l_state[12] = {false};
    bool *curr_sw = NULL;

    void paint_bus_w(QPainter &painter);
    void paint_state(QPainter &painter);
    void paint_q(QPainter &painter);
    void paint_p(QPainter &painter);
    void paint_alarm(QPainter &painter);
    void paint_clock(QPainter &painter);
    void paint_sw_u(QPainter &painter);
    void paint_sw_l(QPainter &painter);
    void paint_rotary(QPainter &painter);

    int calculate_rotary_pos(QPoint &m);
    bool check_ignition(QPoint &m);
    bool check_rotary(QPoint &m);
    bool check_sw_u(QPoint &m);
    bool check_sw_l(QPoint &m);

protected:
	void paintEvent(QPaintEvent *event);
    QSize sizeHint() const;
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

public:
    explicit ControlPanel(QWidget *parent = nullptr);
    ~ControlPanel();
    QSizePolicy sizePolicy();

public slots:
    void slot_bus_w_changed(uint16_t val);
    void slot_state_changed(int state);
    void slot_reg_changed(int reg, uint16_t val);
    void slot_p_changed(int state);
    void slot_alarm_changed(int state);
    void slot_clock_changed(int state);

signals:
    void signal_start_toggled(bool state);
    void signal_cycle_clicked();
    void signal_clear_clicked();
    void signal_oprq_clicked();
    void signal_clock_toggled(bool state);
};

#endif // CONTROLPANEL_H

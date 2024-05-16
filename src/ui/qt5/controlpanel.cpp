#include <QDebug>
#include <QPainter>
#include <math.h>
#include "controlpanel.h"
#include "ectl.h"

#define DEBUG_RECTS 1

static const int led_u_top = 48;
static const int led_l_top = 179;
static const int led_width = 24;
static const int led_height = 24;

static const QRect led_w_rects[] = {
    QRect( 53, led_u_top, led_width, led_height),
    QRect( 89, led_u_top, led_width, led_height),
    QRect(125, led_u_top, led_width, led_height),
    QRect(161, led_u_top, led_width, led_height),
    QRect(197, led_u_top, led_width, led_height),
    QRect(233, led_u_top, led_width, led_height),
    QRect(269, led_u_top, led_width, led_height),
    QRect(305, led_u_top, led_width, led_height),
    QRect(341, led_u_top, led_width, led_height),
    QRect(377, led_u_top, led_width, led_height),
    QRect(413, led_u_top, led_width, led_height),
    QRect(449, led_u_top, led_width, led_height),
    QRect(484, led_u_top, led_width, led_height),
    QRect(520, led_u_top, led_width, led_height),
    QRect(556, led_u_top, led_width, led_height),
    QRect(592, led_u_top, led_width, led_height)
};

static const QRect led_mode_rect    = QRect( 89, led_l_top, led_width, led_height);
static const QRect led_stopn_rect   = QRect(125, led_l_top, led_width, led_height);
static const QRect led_clock_rect   = QRect(412, led_l_top, led_width, led_height);
static const QRect led_q_rect       = QRect(448, led_l_top, led_width, led_height);
static const QRect led_p_rect       = QRect(483, led_l_top, led_width, led_height);
static const QRect led_mc_rect      = QRect(519, led_l_top, led_width, led_height);
static const QRect led_irq_rect     = QRect(555, led_l_top, led_width, led_height);
static const QRect led_run_rect     = QRect(952, 49, led_width, led_height);
static const QRect led_wait_rect    = QRect(988, 49, led_width, led_height);
static const QRect led_alarm_rect   = QRect(1024, 49, led_width, led_height);
static const QRect led_on_rect      = QRect(997, 121, led_width, led_height);

static const int sw_u_top = 89;
static const int sw_l_top = 220;
static const int sw_width = 36;
static const int sw_height = 48;

static const QRect sw_u_rects[16] = {
    QRect( 47, sw_u_top, sw_width, sw_height),
    QRect( 83, sw_u_top, sw_width, sw_height),
    QRect(118, sw_u_top, sw_width, sw_height),
    QRect(154, sw_u_top, sw_width, sw_height),
    QRect(190, sw_u_top, sw_width, sw_height),
    QRect(226, sw_u_top, sw_width, sw_height),
    QRect(262, sw_u_top, sw_width, sw_height),
    QRect(298, sw_u_top, sw_width, sw_height),
    QRect(334, sw_u_top, sw_width, sw_height),
    QRect(369, sw_u_top, sw_width, sw_height),
    QRect(405, sw_u_top, sw_width, sw_height),
    QRect(441, sw_u_top, sw_width, sw_height),
    QRect(477, sw_u_top, sw_width, sw_height),
    QRect(513, sw_u_top, sw_width, sw_height),
    QRect(549, sw_u_top, sw_width, sw_height),
    QRect(585, sw_u_top, sw_width, sw_height)
};

enum sw_lower_id {SW_STEP, SW_MODE, SW_STOPN, SW_CYCLE, SW_LOAD, SW_STORE, SW_FETCH, SW_START, SW_BIN, SW_CLEAR, SW_CLOCK, SW_OPRQ};

static const struct sw_desc sw_l[12] = {
    { QRect( 46, sw_l_top, sw_width, sw_height), true },
    { QRect( 82, sw_l_top, sw_width, sw_height), false },
    { QRect(118, sw_l_top, sw_width, sw_height), true },
    { QRect(154, sw_l_top, sw_width, sw_height), true },
    { QRect(190, sw_l_top, sw_width, sw_height), true },
    { QRect(226, sw_l_top, sw_width, sw_height), true },
    { QRect(262, sw_l_top, sw_width, sw_height), true },
    { QRect(298, sw_l_top, sw_width, sw_height), false },
    { QRect(333, sw_l_top, sw_width, sw_height), true },
    { QRect(369, sw_l_top, sw_width, sw_height), true },
    { QRect(405, sw_l_top, sw_width, sw_height), false },
    { QRect(585, sw_l_top, sw_width, sw_height), true }
};

static const QRect rotary_rect = QRect(708, 81, 150, 150);
static const QPoint rotary_center = QPoint(784, 159);
static const int rotary_radius = 65;

static const QRect ignition_rect = QRect(948, 161, 103, 116);
static const QPoint ignition_center = QPoint(1010, 226);
static const int ignition_radius = 36;

// -----------------------------------------------------------------------
ControlPanel::ControlPanel(QWidget *parent) :
    QWidget(parent)
{
    for (int p=0 ; p<16 ; p++) {
        QString filename = QString("://pulpit/%1.png").arg(p, 2, 10, QLatin1Char('0'));
        plane[p].load(filename);
    }

    width = plane[0].width();
    height = plane[0].height();
}

// -----------------------------------------------------------------------
ControlPanel::~ControlPanel()
{

}

// -----------------------------------------------------------------------
void ControlPanel::paint_bus_w(QPainter &painter)
{
    for (int i=0 ; i<16 ; i++) {
        if (w & (1 << (15-i))) {
            painter.drawPixmap(led_w_rects[i].x(), led_w_rects[i].y(), plane[1].copy(led_w_rects[i]));
        }
    }
}

// -----------------------------------------------------------------------
void ControlPanel::paint_state(QPainter &painter)
{
    switch (state) {
    case ECTL_STATE_RUN:
        painter.drawPixmap(led_run_rect.x(), led_run_rect.y(), plane[1].copy(led_run_rect));
        break;
    case ECTL_STATE_WAIT:
        painter.drawPixmap(led_wait_rect.x(), led_wait_rect.y(), plane[1].copy(led_wait_rect));
        break;
    }
}

// -----------------------------------------------------------------------
void ControlPanel::paint_q(QPainter &painter)
{
    if (q) {
        painter.drawPixmap(led_q_rect.x(), led_q_rect.y(), plane[1].copy(led_q_rect));
    }
}

// -----------------------------------------------------------------------
void ControlPanel::paint_sw_u(QPainter &painter)
{
    for (int i=0 ; i<16 ; i++) {
        if (sw_u_state[i]) {
            painter.drawPixmap(sw_u_rects[i].x(), sw_u_rects[i].y(), plane[1].copy(sw_u_rects[i]));
        }
    }
}

// -----------------------------------------------------------------------
void ControlPanel::paint_sw_l(QPainter &painter)
{
    for (int i=SW_STEP ; i<=SW_OPRQ ; i++) {
        if (sw_l_state[i]) {
            painter.drawPixmap(sw_l[i].r.x(), sw_l[i].r.y(), plane[1].copy(sw_l[i].r));
        }
    }
}
// -----------------------------------------------------------------------
void ControlPanel::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);

    painter.drawPixmap(0, 0, plane[0]);
    painter.drawPixmap(led_on_rect.x(), led_on_rect.y(), plane[1].copy(led_on_rect));
    paint_bus_w(painter);
    paint_state(painter);
    paint_q(painter);
    paint_sw_u(painter);
    paint_sw_l(painter);

    if (DEBUG_RECTS) {
        QPen pen = QPen(Qt::DotLine);
        pen.setColor(QColor(0, 255, 0));
        painter.setPen(pen);
        for (int i=0; i<12 ; i++) painter.drawRect(sw_l[i].r);
        for (int i=0; i<16 ; i++) painter.drawRect(sw_u_rects[i]);
        for (int i=0; i<16 ; i++) painter.drawRect(led_w_rects[i]);
        painter.drawRect(rotary_rect);
        painter.drawRect(ignition_rect);
        painter.drawRect(led_mode_rect);
        painter.drawRect(led_stopn_rect);
        painter.drawRect(led_clock_rect);
        painter.drawRect(led_q_rect);
        painter.drawRect(led_p_rect);
        painter.drawRect(led_mc_rect);
        painter.drawRect(led_irq_rect);
        painter.drawRect(led_on_rect);
        painter.drawRect(led_run_rect);
        painter.drawRect(led_wait_rect);
        painter.drawRect(led_alarm_rect);
    }

    painter.end();
}

// -----------------------------------------------------------------------
QSize ControlPanel::sizeHint() const
{
    return QSize(width, height);
}

// -----------------------------------------------------------------------
QSizePolicy ControlPanel::sizePolicy()
{
    return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

// --------------------------------------------------------------------------
void ControlPanel::mousePressEvent(QMouseEvent *event)
{
    QPoint m = event->pos();
    // ignition
    if (m.x() > ignition_rect.x()) {
        int distance = sqrt(pow(ignition_center.x() - m.x(), 2) + pow(ignition_center.y() - m.y(), 2));
        if (distance <= ignition_radius) {
            qDebug() << "in ignition";
        }
    // rotary switch
    } else if (m.x() > rotary_rect.x()) {
        int distance = sqrt(pow(rotary_center.x() - m.x(), 2) + pow(rotary_center.y() - m.y(), 2));
        if (distance <= rotary_radius) {
            qDebug() << "in rotary";
        }
    // switches - upper row
    } else if ((m.y() > sw_u_top) && (m.y() < sw_u_top + sw_height)) {
        qDebug() << "upper row";
        for (int sw=0 ; sw<16 ; sw++) {
            if (m.y() >= sw_u_rects[sw].y()
                && m.y() <= sw_u_rects[sw].y() + sw_height
                && m.x() >= sw_u_rects[sw].x()
                && m.x() <= sw_u_rects[sw].x() + sw_width
            ) {
                qDebug() << "upper switch " << sw;
                sw_u_state[sw] = !sw_u_state[sw];
                break;
            }
        }
    // switches - lower row
    } else if ((m.y() > sw_l_top) && (m.y() < sw_l_top + sw_height)) {
        qDebug() << "lower row";
        for (int sw=SW_STEP ; sw<=SW_OPRQ ; sw++) {
            if (m.y() >= sw_l[sw].r.y()
                && m.y() <= sw_l[sw].r.y() + sw_height
                && m.x() >= sw_l[sw].r.x()
                && m.x() <= sw_l[sw].r.x() + sw_width
                ) {
                qDebug() << "lower switch " << sw;
                if (sw_l[sw].momentary) {
                    sw_l_state[sw] = true;
                    curr_sw = sw_l_state + sw;
                } else {
                    sw_l_state[sw] = !sw_l_state[sw];
                }
                break;
            }
        }
    }
    update();
}

// --------------------------------------------------------------------------
void ControlPanel::mouseReleaseEvent(QMouseEvent *event)
{
    if (curr_sw) {
        *curr_sw = false;
        curr_sw = NULL;
    }
    update();
}

// -----------------------------------------------------------------------
void ControlPanel::cpu_bus_w_changed(uint16_t val)
{
    w = val;
    update();
}

// -----------------------------------------------------------------------
void ControlPanel::cpu_state_changed(int new_state)
{
    state = new_state;
    update();
}

// -----------------------------------------------------------------------
void ControlPanel::cpu_reg_changed(int reg, uint16_t val)
{
    if (reg == ECTL_REG_SR) {
        q = val & 0b100000;
    }
    update();
}

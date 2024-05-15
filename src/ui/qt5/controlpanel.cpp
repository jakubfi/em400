#include <QDebug>
#include <QPainter>
#include "controlpanel.h"
#include "ectl.h"

static const int led_h_top = 52;
static const int led_l_top = 183;
static const int led_width = 24;
static const int led_height = 24;

static const QRect led_w_rects[] = {
    QRect( 54, led_h_top, led_width, led_height),
    QRect( 90, led_h_top, led_width, led_height),
    QRect(126, led_h_top, led_width, led_height),
    QRect(162, led_h_top, led_width, led_height),
    QRect(198, led_h_top, led_width, led_height),
    QRect(233, led_h_top, led_width, led_height),
    QRect(269, led_h_top, led_width, led_height),
    QRect(305, led_h_top, led_width, led_height),
    QRect(341, led_h_top, led_width, led_height),
    QRect(377, led_h_top, led_width, led_height),
    QRect(413, led_h_top, led_width, led_height),
    QRect(449, led_h_top, led_width, led_height),
    QRect(484, led_h_top, led_width, led_height),
    QRect(520, led_h_top, led_width, led_height),
    QRect(556, led_h_top, led_width, led_height),
    QRect(592, led_h_top, led_width, led_height)
};

static const QRect led_mode_rect    = QRect( 88, led_l_top, led_width, led_height);
static const QRect led_stopn_rect   = QRect(125, led_l_top, led_width, led_height);
static const QRect led_clock_rect   = QRect(412, led_l_top, led_width, led_height);
static const QRect led_q_rect       = QRect(448, led_l_top, led_width, led_height);
static const QRect led_p_rect       = QRect(483, led_l_top, led_width, led_height);
static const QRect led_mc_rect      = QRect(519, led_l_top, led_width, led_height);
static const QRect led_irq_rect     = QRect(555, led_l_top, led_width, led_height);
static const QRect led_run_rect     = QRect(951, 54, led_width, led_height);
static const QRect led_wait_rect    = QRect(987, 54, led_width, led_height);
static const QRect led_alarm_rect   = QRect(1023, 54, led_width, led_height);
static const QRect led_on_rect      = QRect(997, 125, led_width, led_height);

static const int sw_h_top = 93;
static const int sw_l_top = 224;
static const int sw_width = 36;
static const int sw_height = 48;

static const QRect sw_kb_rects[] = {
    QRect( 47, sw_h_top, sw_width, sw_height),
    QRect( 83, sw_h_top, sw_width, sw_height),
    QRect(118, sw_h_top, sw_width, sw_height),
    QRect(154, sw_h_top, sw_width, sw_height),
    QRect(190, sw_h_top, sw_width, sw_height),
    QRect(226, sw_h_top, sw_width, sw_height),
    QRect(262, sw_h_top, sw_width, sw_height),
    QRect(298, sw_h_top, sw_width, sw_height),
    QRect(334, sw_h_top, sw_width, sw_height),
    QRect(369, sw_h_top, sw_width, sw_height),
    QRect(405, sw_h_top, sw_width, sw_height),
    QRect(441, sw_h_top, sw_width, sw_height),
    QRect(477, sw_h_top, sw_width, sw_height),
    QRect(513, sw_h_top, sw_width, sw_height),
    QRect(549, sw_h_top, sw_width, sw_height),
    QRect(585, sw_h_top, sw_width, sw_height)
};

static const QRect sw_step_rect     = QRect( 46, sw_l_top, sw_width, sw_height);
static const QRect sw_mode_rect     = QRect( 82, sw_l_top, sw_width, sw_height);
static const QRect sw_stopn_rect    = QRect(118, sw_l_top, sw_width, sw_height);
static const QRect sw_cycle_rect    = QRect(154, sw_l_top, sw_width, sw_height);
static const QRect sw_load_rect     = QRect(190, sw_l_top, sw_width, sw_height);
static const QRect sw_store_rect    = QRect(226, sw_l_top, sw_width, sw_height);
static const QRect sw_fetch_rect    = QRect(262, sw_l_top, sw_width, sw_height);
static const QRect sw_start_rect    = QRect(298, sw_l_top, sw_width, sw_height);
static const QRect sw_bin_rect      = QRect(333, sw_l_top, sw_width, sw_height);
static const QRect sw_clear_rect    = QRect(369, sw_l_top, sw_width, sw_height);
static const QRect sw_clock_rect    = QRect(405, sw_l_top, sw_width, sw_height);
static const QRect sw_oprq_rect     = QRect(585, sw_l_top, sw_width, sw_height);

static const QRect rotary_rect = QRect(708, 85, 150, 150);
static const QRect ignition_rect = QRect(948, 165, 103, 116);


// -----------------------------------------------------------------------
ControlPanel::ControlPanel(QWidget *parent) :
    QWidget(parent)
{
    bg.load("://pulpit/00.png");
    fg.load("://pulpit/01.png");

    width = bg.width();
    height = bg.height();
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
            painter.drawPixmap(led_w_rects[i].x(), led_w_rects[i].y(), fg.copy(led_w_rects[i]));
        }
    }
}

// -----------------------------------------------------------------------
void ControlPanel::paint_state(QPainter &painter)
{
    switch (state) {
    case ECTL_STATE_RUN:
        painter.drawPixmap(led_run_rect.x(), led_run_rect.y(), fg.copy(led_run_rect));
        break;
    case ECTL_STATE_WAIT:
        painter.drawPixmap(led_wait_rect.x(), led_wait_rect.y(), fg.copy(led_wait_rect));
        break;
    }
}

// -----------------------------------------------------------------------
void ControlPanel::paint_q(QPainter &painter)
{
    if (q) {
        painter.drawPixmap(led_q_rect.x(), led_q_rect.y(), fg.copy(led_q_rect));
    }
}

// -----------------------------------------------------------------------
void ControlPanel::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);

    painter.drawPixmap(0, 0, bg);
    painter.drawPixmap(led_on_rect.x(), led_on_rect.y(), fg.copy(led_on_rect));
    paint_bus_w(painter);
    paint_state(painter);
    paint_q(painter);

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
    update();
}

// --------------------------------------------------------------------------
void ControlPanel::mouseReleaseEvent(QMouseEvent *event)
{
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

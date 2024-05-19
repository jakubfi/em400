#include <QDebug>
#include <QPainter>
#include <QLineF>
#include <math.h>
#include "controlpanel.h"
#include "ectl.h"

static const int led_u_top = 48;
static const int led_l_top = 179;
static const int led_width = 24;
static const int led_height = 24;

static const QRect led_data[] = {
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
    QRect(592, led_u_top, led_width, led_height),
    QRect( 89, led_l_top, led_width, led_height),
    QRect(125, led_l_top, led_width, led_height),
    QRect(412, led_l_top, led_width, led_height),
    QRect(448, led_l_top, led_width, led_height),
    QRect(483, led_l_top, led_width, led_height),
    QRect(519, led_l_top, led_width, led_height),
    QRect(555, led_l_top, led_width, led_height),
    QRect(952, 49, led_width, led_height),
    QRect(988, 49, led_width, led_height),
    QRect(1024, 49, led_width, led_height),
    QRect(997, 121, led_width, led_height),
};

static const int sw_u_top = 89;
static const int sw_l_top = 220;
static const int sw_width = 36;
static const int sw_height = 48;

#define SND_S "qrc:/sounds/switches/n-"

static const struct sw_desc sw_data[] = {
    { QRect( 47, sw_u_top, sw_width, sw_height), false, SND_S "00-1.wav", SND_S "00-0.wav" },
    { QRect( 83, sw_u_top, sw_width, sw_height), false, SND_S "01-1.wav", SND_S "01-0.wav" },
    { QRect(118, sw_u_top, sw_width, sw_height), false, SND_S "02-1.wav", SND_S "02-0.wav" },
    { QRect(154, sw_u_top, sw_width, sw_height), false, SND_S "03-1.wav", SND_S "03-0.wav" },
    { QRect(190, sw_u_top, sw_width, sw_height), false, SND_S "04-1.wav", SND_S "04-0.wav" },
    { QRect(226, sw_u_top, sw_width, sw_height), false, SND_S "05-1.wav", SND_S "05-0.wav" },
    { QRect(262, sw_u_top, sw_width, sw_height), false, SND_S "06-1.wav", SND_S "06-0.wav" },
    { QRect(298, sw_u_top, sw_width, sw_height), false, SND_S "07-1.wav", SND_S "07-0.wav" },
    { QRect(334, sw_u_top, sw_width, sw_height), false, SND_S "08-1.wav", SND_S "08-0.wav" },
    { QRect(369, sw_u_top, sw_width, sw_height), false, SND_S "09-1.wav", SND_S "09-0.wav" },
    { QRect(405, sw_u_top, sw_width, sw_height), false, SND_S "10-1.wav", SND_S "10-0.wav" },
    { QRect(441, sw_u_top, sw_width, sw_height), false, SND_S "11-1.wav", SND_S "11-0.wav" },
    { QRect(477, sw_u_top, sw_width, sw_height), false, SND_S "12-1.wav", SND_S "12-0.wav" },
    { QRect(513, sw_u_top, sw_width, sw_height), false, SND_S "13-1.wav", SND_S "13-0.wav" },
    { QRect(549, sw_u_top, sw_width, sw_height), false, SND_S "14-1.wav", SND_S "14-0.wav" },
    { QRect(585, sw_u_top, sw_width, sw_height), false, SND_S "15-1.wav", SND_S "15-0.wav" },
    { QRect( 46, sw_l_top, sw_width, sw_height), true,  SND_S "step-1.wav",  SND_S "step-0.wav" },
    { QRect( 82, sw_l_top, sw_width, sw_height), false, SND_S "mode-1.wav",  SND_S "mode-0.wav" },
    { QRect(118, sw_l_top, sw_width, sw_height), true,  SND_S "stopn-1.wav", SND_S "stopn-0.wav" },
    { QRect(154, sw_l_top, sw_width, sw_height), true,  SND_S "cycle-1.wav", SND_S "cycle-0.wav" },
    { QRect(190, sw_l_top, sw_width, sw_height), true,  SND_S "load-1.wav",  SND_S "load-0.wav" },
    { QRect(226, sw_l_top, sw_width, sw_height), true,  SND_S "store-1.wav", SND_S "store-0.wav" },
    { QRect(262, sw_l_top, sw_width, sw_height), true,  SND_S "fetch-1.wav", SND_S "fetch-0.wav" },
    { QRect(298, sw_l_top, sw_width, sw_height), false, SND_S "start-1.wav", SND_S "start-0.wav" },
    { QRect(333, sw_l_top, sw_width, sw_height), true,  SND_S "bin-1.wav",   SND_S "bin-0.wav" },
    { QRect(369, sw_l_top, sw_width, sw_height), true,  SND_S "clear-1.wav", SND_S "clear-0.wav" },
    { QRect(405, sw_l_top, sw_width, sw_height), false, SND_S "clock-1.wav", SND_S "clock-0.wav" },
    { QRect(585, sw_l_top, sw_width, sw_height), true,  SND_S "oprq-1.wav",  SND_S "oprq-0.wav" }
};

static const QRect rotary_rect = QRect(669, 41, 232, 232);

#define SND_R "qrc:/sounds/rotary/"

static const QUrl rotary_sounds_r[] = {
    QUrl(SND_R "r00.wav"),
    QUrl(SND_R "r01.wav"),
    QUrl(SND_R "r02.wav"),
    QUrl(SND_R "r03.wav"),
    QUrl(SND_R "r04.wav"),
    QUrl(SND_R "r05.wav"),
    QUrl(SND_R "r06.wav"),
    QUrl(SND_R "r07.wav"),
    QUrl(SND_R "r08.wav"),
    QUrl(SND_R "r09.wav"),
    QUrl(SND_R "r10.wav"),
    QUrl(SND_R "r11.wav"),
    QUrl(SND_R "r12.wav"),
    QUrl(SND_R "r13.wav"),
    QUrl(SND_R "r14.wav"),
    QUrl(SND_R "r15.wav"),
};

static const QUrl rotary_sounds_l[] = {
    QUrl(SND_R "l00.wav"),
    QUrl(SND_R "l01.wav"),
    QUrl(SND_R "l02.wav"),
    QUrl(SND_R "l03.wav"),
    QUrl(SND_R "l04.wav"),
    QUrl(SND_R "l05.wav"),
    QUrl(SND_R "l06.wav"),
    QUrl(SND_R "l07.wav"),
    QUrl(SND_R "l08.wav"),
    QUrl(SND_R "l09.wav"),
    QUrl(SND_R "l10.wav"),
    QUrl(SND_R "l11.wav"),
    QUrl(SND_R "l12.wav"),
    QUrl(SND_R "l13.wav"),
    QUrl(SND_R "l14.wav"),
    QUrl(SND_R "l15.wav"),
};

static const QRect ignition_rect = QRect(974, 184, 70, 70);
static const QPoint ignition_center = QPoint(1010, 226);
static const int ignition_radius = 36;

#define SND_I "qrc:/sounds/ignition/"

static const QUrl ignition_sounds_r[] = {
    QUrl(SND_I "off-l.wav"), // never happen
    QUrl(SND_I "on-r.wav"),
    QUrl(SND_I "lock-r.wav"),
};

static const QUrl ignition_sounds_l[] = {
    QUrl(SND_I "off-l.wav"),
    QUrl(SND_I "on-l.wav"),
    QUrl(SND_I "lock-r.wav"), // never happen
};

// -----------------------------------------------------------------------
ControlPanel::ControlPanel(QWidget *parent):
    QWidget(parent)
{
    for (int p=0 ; p<16 ; p++) {
        QString filename = QString("://pulpit/%1.png").arg(p, 2, 10, QLatin1Char('0'));
        plane[p].load(filename);
    }

    width = plane[0].width();
    height = plane[0].height();

    int i=0;
    for (auto const& sw_d : sw_data) {
        sw[i] = new Switch(plane[1].copy(sw_d.r), sw_d.snd_on, sw_d.snd_off, sw_d.momentary, this);
        sw[i]->move(sw_d.r.topLeft());
        i++;
    }

    i=0;
    for (auto const& led_d : led_data) {
        led[i] = new LED(plane[1].copy(led_d), this);
        led[i]->move(led_d.topLeft());
        i++;
    }

    QPixmap gfx[16];
    for (i=0 ; i<16 ; i++) {
        gfx[i] = plane[i].copy(rotary_rect);
    }
    rotary = new Rotary(gfx, rotary_sounds_r, rotary_sounds_l, this);
    rotary->move(rotary_rect.topLeft());

    for (i=0 ; i<3 ; i++) {
        gfx[i] = plane[i].copy(ignition_rect);
    }
    ignition = new Ignition(gfx, ignition_sounds_r, ignition_sounds_l, this);
    ignition->move(ignition_rect.topLeft());

    led[LED_ON]->set(true);
}

// -----------------------------------------------------------------------
ControlPanel::~ControlPanel()
{
    for (int i=0 ; i<SW_CNT ; i++) delete sw[i];
    for (int i=0 ; i<LED_CNT ; i++) delete led[i];
    delete rotary;
    delete ignition;
}

// -----------------------------------------------------------------------
void ControlPanel::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);

    painter.drawPixmap(0, 0, plane[0]);

    if (false) {
        QPen pen = QPen(Qt::DotLine);
        pen.setColor(QColor(0, 255, 0));
        painter.setPen(pen);
        painter.drawRect(ignition_rect);
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
bool ControlPanel::check_ignition(QPoint &m)
{
    if (m.x() > ignition_rect.x()) {
        int distance = sqrt(pow(ignition_center.x() - m.x(), 2) + pow(ignition_center.y() - m.y(), 2));
        if (distance <= ignition_radius) {
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------
void ControlPanel::slot_bus_w_changed(uint16_t val)
{
    for (int i=0 ; i<16 ; i++) {
        bool state = val & (1<<(15-i));
        led[i]->set(state);
    }
}

// -----------------------------------------------------------------------
void ControlPanel::slot_state_changed(int state)
{
    switch (state) {
    case ECTL_STATE_RUN:
        led[LED_RUN]->set(true);
        led[LED_WAIT]->set(false);
        break;
    case ECTL_STATE_WAIT:
        led[LED_RUN]->set(false);
        led[LED_WAIT]->set(true);
        break;
    default:
        led[LED_RUN]->set(false);
        led[LED_WAIT]->set(false);
        break;
    }
}

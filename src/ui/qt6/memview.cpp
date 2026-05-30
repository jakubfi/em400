#include <cctype>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QLabel>
#include <emcrk/r40.h>
#include "memview.h"
#include "libem400.h"

// -----------------------------------------------------------------------
static QPushButton *make_toggle_btn(const QString &text, bool checked = false)
{
	QPushButton *b = new QPushButton(text);
	b->setCheckable(true);
	b->setChecked(checked);
	b->setFlat(true);
	b->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	return b;
}

// -----------------------------------------------------------------------
MemView::MemView(QWidget *parent) : QWidget(parent)
{
	bottom = 100;
	right = 100;
	words_per_line = 16;

	set_font("Monospace", 12);

	setFocusPolicy(Qt::WheelFocus);

	// header bar
	header = new QWidget(this);
	QHBoxLayout *hlay = new QHBoxLayout(header);
	hlay->setContentsMargins(4, 2, 4, 2);
	hlay->setSpacing(4);

	hlay->addWidget(new QLabel("Block:"));
	nb_spin = new QSpinBox();
	nb_spin->setRange(0, 15);
	nb_spin->setValue(0);
	nb_spin->setFixedWidth(48);
	hlay->addWidget(nb_spin);
	hlay->addSpacing(8);

	btn_hex  = make_toggle_btn("HEX",  true);
	btn_udec = make_toggle_btn("+DEC", false);
	btn_sdec = make_toggle_btn("-DEC", false);
	hlay->addWidget(btn_hex);
	hlay->addWidget(btn_udec);
	hlay->addWidget(btn_sdec);
	hlay->addSpacing(8);

	btn_ascii = make_toggle_btn("ASCII", true);
	btn_r40   = make_toggle_btn("R40",   false);
	hlay->addWidget(btn_ascii);
	hlay->addWidget(btn_r40);
	hlay->addStretch();

	content_top = header->sizeHint().height();
	if (content_top <= 0) content_top = 28;
	header->setFixedHeight(content_top);
	header->setGeometry(0, 0, width(), content_top);

	// scrollbar
	scroll = new QScrollBar(Qt::Vertical, this);
	scroll->setMinimum(0);
	scroll->setSingleStep(1);
	scroll->setVisible(false);
	update_scroll_range();

	// connections
	connect(scroll, &QScrollBar::valueChanged, this, &MemView::update_contents_no_nb);

	connect(nb_spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int nb) {
		if (!cpu_running) {
			cnb = nb;
			update();
		}
	});

	connect(btn_hex,  &QPushButton::clicked, this, [this]() { set_format(FMT_HEX); });
	connect(btn_udec, &QPushButton::clicked, this, [this]() { set_format(FMT_UDEC); });
	connect(btn_sdec, &QPushButton::clicked, this, [this]() { set_format(FMT_SDEC); });

	connect(btn_ascii, &QPushButton::clicked, this, [this]() { toggle_panel(PANEL_ASCII); });
	connect(btn_r40,   &QPushButton::clicked, this, [this]() { toggle_panel(PANEL_R40); });
}

// -----------------------------------------------------------------------
MemView::~MemView() {}

// -----------------------------------------------------------------------
void MemView::connect_emu(EmuModel *emu)
{
	e = emu;
	connect(e, &EmuModel::signal_state_changed, this, &MemView::slot_state_changed);
	connect(e, &EmuModel::signal_reg_changed,   this, &MemView::slot_reg_changed);
}

// -----------------------------------------------------------------------
void MemView::slot_state_changed(int state)
{
	cpu_running = (state == EM400_STATE_RUN);
	if (cpu_running) {
		int qnb = e->get_qnb();
		if (qnb != cnb) {
			cnb = qnb;
			QSignalBlocker blk(nb_spin);
			nb_spin->setValue(cnb);
		}
	}
	nb_spin->setReadOnly(cpu_running);
	nb_spin->setButtonSymbols(cpu_running
		? QAbstractSpinBox::NoButtons
		: QAbstractSpinBox::UpDownArrows);
	update();
}

// -----------------------------------------------------------------------
void MemView::slot_reg_changed(int reg, uint16_t)
{
	if (cpu_running && reg == EM400_REG_SR) {
		int qnb = e->get_qnb();
		if (qnb != cnb) {
			cnb = qnb;
			QSignalBlocker blk(nb_spin);
			nb_spin->setValue(cnb);
		}
	}
	update();
}

// -----------------------------------------------------------------------
int MemView::val_chars() const
{
	switch (fmt) {
		case FMT_HEX:  return 5; // "xxxx "
		case FMT_UDEC: return 6; // "ddddd "
		case FMT_SDEC: return 7; // "-ddddd "
	}
	return 5;
}

// -----------------------------------------------------------------------
int MemView::panel_chars() const
{
	switch (panel) {
		case PANEL_OFF:   return 0;
		case PANEL_ASCII: return 2;
		case PANEL_R40:   return 3;
	}
	return 0;
}

// -----------------------------------------------------------------------
int MemView::compute_words_per_line() const
{
	int scroll_w = scroll ? scroll->sizeHint().width() : 15;
	int avail = right - mem_x_start - scroll_w;
	if (panel != PANEL_OFF) avail -= font_width; // gap before side panel divider
	int chars_per_word = val_chars() + panel_chars();
	int max_wpl = avail / (chars_per_word * font_width);
	if (max_wpl < 1) return 1;
	int wpl = 1;
	while (wpl * 2 <= max_wpl) wpl *= 2;
	return wpl;
}

// -----------------------------------------------------------------------
QSize MemView::sizeHint() const
{
	// Express a real preference so adjustSize() gives the view a usable size
	// instead of collapsing it: a comfortable words-per-line and a screenful of
	// rows. Without this the default QWidget hint is tiny and the whole window
	// shrinks to a sliver when the debugger is shown.
	const int pref_wpl = 16;
	const int pref_lines = 16;
	int scroll_w = scroll ? scroll->sizeHint().width() : 15;
	int pixels_per_word = (val_chars() + panel_chars()) * font_width;
	int w = mem_x_start + pref_wpl * pixels_per_word + scroll_w;
	if (panel != PANEL_OFF) w += font_width; // gap before side panel divider
	int h = content_top + col_hdr_h + pref_lines * line_height;
	return QSize(w, h);
}

// -----------------------------------------------------------------------
void MemView::apply_wpl_change()
{
	int new_wpl = compute_words_per_line();
	if (new_wpl != words_per_line) {
		words_per_line = new_wpl;
		caddr = (caddr / words_per_line) * words_per_line;
	}
	QSignalBlocker blk(scroll);
	update_scroll_range();
	scroll->setValue(caddr / words_per_line);
}

// -----------------------------------------------------------------------
void MemView::set_format(DisplayFormat f)
{
	if (fmt == f) return;
	fmt = f;
	btn_hex->setChecked(fmt == FMT_HEX);
	btn_udec->setChecked(fmt == FMT_UDEC);
	btn_sdec->setChecked(fmt == FMT_SDEC);
	apply_wpl_change();
	update();
}

// -----------------------------------------------------------------------
void MemView::toggle_panel(SidePanel p)
{
	panel = (panel == p) ? PANEL_OFF : p;
	btn_ascii->setChecked(panel == PANEL_ASCII);
	btn_r40->setChecked(panel == PANEL_R40);
	apply_wpl_change();
	update();
}

// -----------------------------------------------------------------------
void MemView::set_font(QString name, int size)
{
	font.setFamily(name);
	if (size > 0) font.setPixelSize(size);
	setFont(font);
	QFontMetrics fm(font);
	font_height = fm.lineSpacing();
	font_width = fm.horizontalAdvance('9');
	update_font_related_dimensions();
}

// -----------------------------------------------------------------------
void MemView::update_font_related_dimensions()
{
	const int interline = 4;
	half_font_width = font_width / 2;
	line_height = font_height + interline;
	col_hdr_h = line_height;

	addr_x_start = half_font_width;
	addr_y_start = col_hdr_h + font_height;
	addr_len = 5;

	mem_x_start = addr_x_start + font_width * (addr_len + 1);
	mem_y_start = addr_y_start;

	divider_x_pos = mem_x_start - font_width;
}

// -----------------------------------------------------------------------
void MemView::update_scroll_range()
{
	const int mem_lines = 0x10000 / words_per_line;
	const int visible_lines = (bottom - col_hdr_h) / line_height;
	int max_top_line = mem_lines - visible_lines;
	if (max_top_line < 0) max_top_line = 0;
	scroll->setMaximum(max_top_line);
}

// -----------------------------------------------------------------------
void MemView::update_contents(int new_nb, int new_addr)
{
	cnb = new_nb;
	caddr = (new_addr / words_per_line) * words_per_line;
	QSignalBlocker blk(nb_spin);
	nb_spin->setValue(cnb);
	QSignalBlocker blk2(scroll);
	scroll->setValue(caddr / words_per_line);
	update();
}

// -----------------------------------------------------------------------
void MemView::update_contents_no_nb(int new_line)
{
	int new_addr = new_line * words_per_line;
	if (new_addr == caddr) return;
	caddr = new_addr;
	update();
}

// -----------------------------------------------------------------------
void MemView::paintEvent(QPaintEvent *event)
{
	// NOTE: when cell selection/editing is added, split into
	// draw_offset_row(), draw_line(), draw_cell() helpers.

	QPainter painter(this);
	painter.translate(0, content_top);
	painter.setClipRect(0, 0, right + 1, bottom + 1);

	painter.fillRect(0, 0, right, bottom, palette().color(QPalette::Base));

	if (e) {
		int cell_w = val_chars() * font_width;

		// column offset header row
		font.setBold(true);
		painter.setFont(font);
		painter.setPen(palette().color(QPalette::Text));
		for (int x = 0; x < words_per_line; x++) {
			QString off_str = QString("%1").arg(x, val_chars() - 1, 16, QLatin1Char(' '));
			painter.drawText(mem_x_start + x * cell_w, font_height, off_str);
		}
		painter.setPen(palette().color(QPalette::Highlight));
		painter.drawLine(divider_x_pos, col_hdr_h, right - 1, col_hdr_h);
		int pcell_w = panel_chars() * font_width;
		int side_x = (panel != PANEL_OFF)
			? mem_x_start + words_per_line * cell_w + font_width
			: 0;

		for (int y = 0; y < total_lines; y++) {
			int base_addr = caddr + y * words_per_line;
			if (base_addr > 0xffff) break;

			// address
			QString addr_str = QString("%1").arg((uint16_t)base_addr, 4, 16, QLatin1Char('0'));
			font.setBold(true);
			painter.setFont(font);
			painter.setPen(palette().color(QPalette::Text));
			painter.drawText(addr_x_start, addr_y_start + y * line_height, addr_str);

			font.setBold(false);
			painter.setFont(font);

			for (int x = 0; x < words_per_line; x++) {
				int addr = base_addr + x;
				if (addr > 0xffff) break;
				int val = e->get_mem(cnb, addr);

				// value
				QString val_str;
				if (val >= 0) {
					switch (fmt) {
						case FMT_HEX:
							val_str = QString("%1").arg((uint16_t)val, 4, 16, QLatin1Char('0'));
							break;
						case FMT_UDEC:
							val_str = QString("%1").arg((uint16_t)val, 5, 10, QLatin1Char(' '));
							break;
						case FMT_SDEC:
							val_str = QString("%1").arg((int16_t)val, 6, 10, QLatin1Char(' '));
							break;
					}
				} else {
					switch (fmt) {
						case FMT_HEX:  val_str = "----";   break;
						case FMT_UDEC: val_str = "-----";  break;
						case FMT_SDEC: val_str = "------"; break;
					}
				}
				painter.setPen(palette().color(QPalette::Text));
				painter.drawText(mem_x_start + x * cell_w, mem_y_start + y * line_height, val_str);

				// side panel
				if (panel != PANEL_OFF) {
					QString pstr;
					if (panel == PANEL_ASCII) {
						if (val >= 0) {
							unsigned char hi = (val >> 8) & 0xff;
							unsigned char lo = val & 0xff;
							pstr = QString(QChar(isprint(hi) ? hi : '.'))
							     + QString(QChar(isprint(lo) ? lo : '.'));
						} else {
							pstr = "..";
						}
					} else { // R40
						if (val >= 0) {
							uint16_t word = (uint16_t)val;
							char buf[4];
							pstr = r40_to_ascii(&word, 1, buf)
								? QString::fromLatin1(buf, 3)
								: "???";
						} else {
							pstr = "???";
						}
					}
					painter.setPen(palette().color(QPalette::PlaceholderText));
					painter.drawText(side_x + x * pcell_w, mem_y_start + y * line_height, pstr);
				}
			}
		}

		// divider between values and side panel
		if (panel != PANEL_OFF) {
			int sdiv_x = mem_x_start + words_per_line * cell_w + half_font_width;
			painter.setPen(palette().color(QPalette::Highlight));
			painter.drawLine(sdiv_x, 0, sdiv_x, bottom);
		}
	}

	// divider between addr and values
	painter.setPen(palette().color(QPalette::Highlight));
	painter.drawLine(divider_x_pos, 0, divider_x_pos, bottom);

	// frame around content area
	painter.setPen(palette().color(QPalette::Mid));
	painter.drawRect(0, 0, right - 1, bottom - 1);
}

// -----------------------------------------------------------------------
void MemView::resizeEvent(QResizeEvent *event)
{
	header->setGeometry(0, 0, width(), content_top);
	bottom = height() - content_top;
	right = width();
	total_lines = (bottom - col_hdr_h) / line_height + 1;

	apply_wpl_change();
	scroll->setPageStep(total_lines);
	scroll->setGeometry(width() - scroll->sizeHint().width(), content_top + col_hdr_h,
		scroll->sizeHint().width(), bottom - col_hdr_h);

	QWidget::resizeEvent(event);
}

// -----------------------------------------------------------------------
int MemView::calculate_scroll_lines(int angleDelta)
{
	const int native_wheel_step = 120;
	const int lines_per_click = 3;
	const int one_line_advance = native_wheel_step / lines_per_click;

	wheel_tick_accumulator += angleDelta;
	int lines = wheel_tick_accumulator / one_line_advance;
	wheel_tick_accumulator %= one_line_advance;

	return lines;
}

// -----------------------------------------------------------------------
void MemView::wheelEvent(QWheelEvent *event)
{
	const int delta_lines = calculate_scroll_lines(event->angleDelta().y());
	scroll->setValue(scroll->value() - delta_lines);
	event->accept();
}

// -----------------------------------------------------------------------
void MemView::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
		case Qt::Key_H:        set_format(FMT_HEX);          break;
		case Qt::Key_U:        set_format(FMT_UDEC);         break;
		case Qt::Key_D:        set_format(FMT_SDEC);         break;
		case Qt::Key_A:        toggle_panel(PANEL_ASCII);    break;
		case Qt::Key_R:        toggle_panel(PANEL_R40);      break;
		case Qt::Key_PageUp:   scroll->setValue(scroll->value() - total_lines); break;
		case Qt::Key_PageDown: scroll->setValue(scroll->value() + total_lines); break;
		case Qt::Key_Home:     scroll->setValue(0);           break;
		case Qt::Key_End:      scroll->setValue(scroll->maximum()); break;
		default:               QWidget::keyPressEvent(event); break;
	}
}

// -----------------------------------------------------------------------
void MemView::enterEvent(QEnterEvent *event)
{
	scroll->setVisible(true);
	QWidget::enterEvent(event);
}

// -----------------------------------------------------------------------
void MemView::leaveEvent(QEvent *event)
{
	scroll->setVisible(false);
	QWidget::leaveEvent(event);
}

#include <cctype>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QLabel>
#include <emcrk/r40.h>
#include "memview.h"
#include "libem400.h"
#include "theme.h"

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
			cancel_edit();
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
		cancel_edit();
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
	cancel_edit();
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
	cancel_edit();
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
	cancel_edit();
	caddr = new_addr;
	update();
}

// -----------------------------------------------------------------------
// Map a widget-space point to an absolute address in the value column.
// Returns false for clicks on the header, address gutter, side panel or
// outside the populated cells.
bool MemView::hit_test_cell(const QPoint &pos, int &addr) const
{
	int cell_w = val_chars() * font_width;

	int cy = pos.y() - content_top - col_hdr_h;
	if (cy < 0) return false;
	int row = cy / line_height;
	if (row >= total_lines) return false;

	int cx = pos.x() - mem_x_start;
	if (cx < 0) return false;
	int col = cx / cell_w;
	if (col >= words_per_line) return false;

	int a = caddr + row * words_per_line + col;
	if (a > 0xffff) return false;

	addr = a;
	return true;
}

// -----------------------------------------------------------------------
void MemView::start_edit(int addr)
{
	if (cpu_running || !e) return;

	// unmapped words read back as -1 and can't be written, so there is
	// nothing to edit there
	int val = e->get_mem(cnb, addr);
	if (val < 0) return;

	edit_buf.clear();
	switch (fmt) {
		case FMT_HEX:
			edit_buf = QString("%1").arg((uint16_t)val, 4, 16, QLatin1Char('0'));
			break;
		case FMT_UDEC:
			edit_buf = QString::number((uint16_t)val);
			break;
		case FMT_SDEC:
			edit_buf = QString::number((int16_t)val);
			break;
	}

	editing = true;
	edit_nb = cnb;
	edit_addr = addr;
	edit_cursor = 0;
	setFocus();
	emit signal_edit_mode_changed(true, edit_insert);
	update();
}

// -----------------------------------------------------------------------
void MemView::cancel_edit()
{
	if (!editing) return;
	editing = false;
	edit_buf.clear();
	emit signal_edit_mode_changed(false, edit_insert);
	update();
}

// -----------------------------------------------------------------------
void MemView::commit_edit()
{
	if (!editing) return;

	bool empty = edit_buf.isEmpty() || edit_buf == "-";
	if (!empty && e) {
		bool ok = false;
		uint16_t v = 0;
		switch (fmt) {
			case FMT_HEX:
				v = (uint16_t)edit_buf.toUInt(&ok, 16);
				break;
			case FMT_UDEC:
				// a leading '-' means a signed value stored as U2
				if (edit_buf.startsWith('-')) {
					v = (uint16_t)(int16_t)edit_buf.toInt(&ok, 10);
				} else {
					v = (uint16_t)edit_buf.toUInt(&ok, 10);
				}
				break;
			case FMT_SDEC:
				v = (uint16_t)(int16_t)edit_buf.toInt(&ok, 10);
				break;
		}
		if (ok) e->set_mem(edit_nb, edit_addr, v);
	}

	editing = false;
	edit_buf.clear();
	emit signal_edit_mode_changed(false, edit_insert);
	update();
}

// -----------------------------------------------------------------------
// Validate a candidate edit buffer against the current format's character
// set and value range. An empty string (or a lone '-') is a valid partial
// edit; it just won't be written on commit.
bool MemView::valid_buf(const QString &s) const
{
	if (s.isEmpty()) return true;

	switch (fmt) {
		case FMT_HEX:
			// no sign in hex: a lone '-' is not a valid partial edit here
			if (s.length() > 4) return false;
			for (QChar c : s) if (!isxdigit((unsigned char)c.toLatin1())) return false;
			return true;
		case FMT_UDEC: {
			if (s == "-") return true;
			// unsigned view, but a leading '-' lets the user enter a
			// signed value that gets stored as its U2 representation
			QString digits = s.startsWith('-') ? s.mid(1) : s;
			for (QChar c : digits) if (!c.isDigit()) return false;
			bool ok = false;
			if (s.startsWith('-')) {
				return s.toInt(&ok) >= -32768 && ok;
			} else {
				return s.length() <= 5 && s.toUInt(&ok) <= 65535 && ok;
			}
		}
		case FMT_SDEC: {
			if (s == "-") return true;
			QString digits = s.startsWith('-') ? s.mid(1) : s;
			for (QChar c : digits) if (!c.isDigit()) return false;
			bool ok = false;
			int v = s.toInt(&ok);
			return ok && v >= -32768 && v <= 32767;
		}
	}
	return false;
}

// -----------------------------------------------------------------------
void MemView::mouseDoubleClickEvent(QMouseEvent *event)
{
	int addr;
	if (event->button() == Qt::LeftButton && hit_test_cell(event->pos(), addr)) {
		start_edit(addr);
		event->accept();
		return;
	}
	QWidget::mouseDoubleClickEvent(event);
}

// -----------------------------------------------------------------------
// Losing keyboard focus while a cell is being edited abandons the edit
// (consistent with Escape); otherwise the overlay and the status-bar
// indicator would stay stuck with no widget receiving keystrokes.
void MemView::focusOutEvent(QFocusEvent *event)
{
	cancel_edit();
	QWidget::focusOutEvent(event);
}

// -----------------------------------------------------------------------
// Format the value column text for one word. val < 0 means the word is
// unreadable (no memory mapped), shown as dashes.
QString MemView::value_text(int val) const
{
	if (val < 0) {
		switch (fmt) {
			case FMT_HEX:  return "----";
			case FMT_UDEC: return "-----";
			case FMT_SDEC: return "------";
		}
		return "----";
	}
	switch (fmt) {
		case FMT_HEX:  return QString("%1").arg((uint16_t)val, 4, 16, QLatin1Char('0'));
		case FMT_UDEC: return QString("%1").arg((uint16_t)val, 5, 10, QLatin1Char(' '));
		case FMT_SDEC: return QString("%1").arg((int16_t)val, 6, 10, QLatin1Char(' '));
	}
	return QString();
}

// -----------------------------------------------------------------------
// Format the side panel text (ASCII pair or R40 triplet) for one word.
QString MemView::panel_text(int val) const
{
	if (panel == PANEL_ASCII) {
		if (val < 0) return "..";
		unsigned char hi = (val >> 8) & 0xff;
		unsigned char lo = val & 0xff;
		return QString(QChar(isprint(hi) ? hi : '.'))
		     + QString(QChar(isprint(lo) ? lo : '.'));
	}
	// R40
	if (val < 0) return "???";
	uint16_t word = (uint16_t)val;
	char buf[4];
	return r40_to_ascii(&word, 1, buf) ? QString::fromLatin1(buf, 3) : "???";
}

// -----------------------------------------------------------------------
// The non-scrolling row of column offsets above the memory dump.
void MemView::draw_offset_row(QPainter &painter, int cell_w)
{
	font.setBold(true);
	painter.setFont(font);
	painter.setPen(palette().color(QPalette::Text));
	for (int x = 0; x < words_per_line; x++) {
		QString off_str = QString("%1").arg(x, val_chars() - 1, 16, QLatin1Char(' '));
		painter.drawText(mem_x_start + x * cell_w, font_height, off_str);
	}
	painter.setPen(QPen(em400_sep_color(palette()), 2));
	painter.drawLine(divider_x_pos, col_hdr_h, right - 1, col_hdr_h);
}

// -----------------------------------------------------------------------
// One memory line: the address gutter plus every word cell (and its side
// panel companion) on that line.
void MemView::draw_line(QPainter &painter, int y, int base_addr, int cell_w, int pcell_w, int side_x)
{
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

		// the cell under edit gets its own overlay and no side panel
		if (editing && cnb == edit_nb && addr == edit_addr) {
			draw_edit_cell(painter, x, y, cell_w);
			continue;
		}

		draw_value_cell(painter, x, y, val, cell_w);
		if (panel != PANEL_OFF) {
			draw_panel_cell(painter, x, y, val, pcell_w, side_x);
		}
	}
}

// -----------------------------------------------------------------------
void MemView::draw_value_cell(QPainter &painter, int x, int y, int val, int cell_w)
{
	painter.setPen(palette().color(QPalette::Text));
	painter.drawText(mem_x_start + x * cell_w, mem_y_start + y * line_height, value_text(val));
}

// -----------------------------------------------------------------------
void MemView::draw_panel_cell(QPainter &painter, int x, int y, int val, int pcell_w, int side_x)
{
	painter.setPen(em400_dim_text_color(palette()));
	painter.drawText(side_x + x * pcell_w, mem_y_start + y * line_height, panel_text(val));
}

// -----------------------------------------------------------------------
// The cell currently being edited: the edit buffer with a caret over a
// highlighted background, drawn in place of the stored value.
void MemView::draw_edit_cell(QPainter &painter, int x, int y, int cell_w)
{
	int ex = mem_x_start + x * cell_w;
	int ey = col_hdr_h + y * line_height;
	int baseline = mem_y_start + y * line_height;

	// highlight hugs the value digits with half a char of padding on each
	// side, instead of spanning the full cell (whose width includes the
	// trailing inter-column gap)
	int val_w = (val_chars() - 1) * font_width;
	painter.fillRect(ex - half_font_width, ey, val_w + font_width, line_height,
		palette().color(QPalette::Highlight));
	painter.setPen(palette().color(QPalette::HighlightedText));
	painter.drawText(ex, baseline, edit_buf);

	int cur_x = ex + edit_cursor * font_width;
	if (edit_insert) {
		// insert caret: thin vertical bar between chars
		painter.fillRect(cur_x, ey + 1, 2, line_height - 2,
			palette().color(QPalette::HighlightedText));
	} else {
		// overwrite caret: solid block over the char, redrawn inverted.
		// Inset top/bottom by 1px so its edge stays visible against the
		// highlighted cell background.
		painter.fillRect(cur_x, ey + 1, font_width, line_height - 2,
			palette().color(QPalette::HighlightedText));
		if (edit_cursor < edit_buf.length()) {
			painter.setPen(palette().color(QPalette::Highlight));
			painter.drawText(cur_x, baseline, QString(edit_buf.at(edit_cursor)));
		}
	}
}

// -----------------------------------------------------------------------
void MemView::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.translate(0, content_top);
	painter.setClipRect(0, 0, right + 1, bottom + 1);

	painter.fillRect(0, 0, right, bottom, palette().color(QPalette::Base));

	if (e) {
		int cell_w = val_chars() * font_width;
		int pcell_w = panel_chars() * font_width;
		int side_x = (panel != PANEL_OFF)
			? mem_x_start + words_per_line * cell_w + font_width
			: 0;

		draw_offset_row(painter, cell_w);

		for (int y = 0; y < total_lines; y++) {
			int base_addr = caddr + y * words_per_line;
			if (base_addr > 0xffff) break;
			draw_line(painter, y, base_addr, cell_w, pcell_w, side_x);
		}

		// divider between values and side panel
		if (panel != PANEL_OFF) {
			int sdiv_x = mem_x_start + words_per_line * cell_w + half_font_width;
			painter.setPen(QPen(em400_sep_color(palette()), 2));
			painter.drawLine(sdiv_x, 0, sdiv_x, bottom);
		}
	}

	// divider between addr and values
	painter.setPen(QPen(em400_sep_color(palette()), 2));
	painter.drawLine(divider_x_pos, 0, divider_x_pos, bottom);
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
	if (editing) {
		switch (event->key()) {
			case Qt::Key_Return:
			case Qt::Key_Enter:
				commit_edit();
				break;
			case Qt::Key_Escape:
				cancel_edit();
				break;
			case Qt::Key_Left:
				if (edit_cursor > 0) { edit_cursor--; update(); }
				break;
			case Qt::Key_Right:
				if (edit_cursor < edit_buf.length()) { edit_cursor++; update(); }
				break;
			case Qt::Key_Insert:
				edit_insert = !edit_insert;
				emit signal_edit_mode_changed(true, edit_insert);
				update();
				break;
			case Qt::Key_Home:
				edit_cursor = 0;
				update();
				break;
			case Qt::Key_End:
				edit_cursor = edit_buf.length();
				update();
				break;
			case Qt::Key_Backspace:
				if (edit_cursor > 0) {
					edit_buf.remove(edit_cursor - 1, 1);
					edit_cursor--;
					update();
				}
				break;
			case Qt::Key_Delete:
				if (edit_cursor < edit_buf.length()) {
					edit_buf.remove(edit_cursor, 1);
					update();
				}
				break;
			default: {
				QString t = event->text();
				if (!t.isEmpty()) {
					QChar c = t.at(0);
					if (fmt == FMT_HEX) c = c.toLower();
					QString cand = edit_buf;
					if (edit_insert) {
						cand.insert(edit_cursor, c);
					} else if (edit_cursor < cand.length()) {
						cand[edit_cursor] = c;
					} else {
						cand.append(c);
					}
					if (valid_buf(cand)) {
						edit_buf = cand;
						edit_cursor++;
						// overwrite: once the field can hold no more digits,
						// keep the caret on the last digit (overwriting it on
						// further input) instead of parking it on the empty
						// slot past the value
						if (!edit_insert && edit_cursor >= edit_buf.length()
								&& !valid_buf(edit_buf + "0")) {
							edit_cursor = edit_buf.length() - 1;
						}
						update();
					}
				}
				break;
			}
		}
		event->accept();
		return;
	}

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

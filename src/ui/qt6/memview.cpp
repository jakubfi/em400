#include <cctype>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QLabel>
#include <QApplication>
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

	// header bar. The widget font is Monospace (set above) and would propagate
	// to these controls; pin the header to the standard UI font instead.
	header = new QWidget(this);
	header->setFont(QApplication::font());
	QHBoxLayout *hlay = new QHBoxLayout(header);
	hlay->setContentsMargins(4, 2, 4, 2);
	hlay->setSpacing(4);

	hlay->addWidget(new QLabel("NB:"));
	nb_spin = new QSpinBox();
	nb_spin->setRange(0, 15);
	nb_spin->setValue(0);
	nb_spin->setFixedWidth(48);
	hlay->addWidget(nb_spin);
	hlay->addSpacing(8);

	btn_hex  = make_toggle_btn("HEX",  true);
	btn_udec = make_toggle_btn("DEC", false);
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
		cancel_edit();
		cnb = nb;
		update();
	});

	connect(btn_hex, &QPushButton::clicked, this, [this]() { set_format(FMT_HEX); });
	connect(btn_udec, &QPushButton::clicked, this, [this]() { set_format(FMT_UDEC); });
	connect(btn_sdec, &QPushButton::clicked, this, [this]() { set_format(FMT_SDEC); });

	connect(btn_ascii, &QPushButton::clicked, this, [this]() { toggle_panel(PANEL_ASCII); });
	connect(btn_r40, &QPushButton::clicked, this, [this]() { toggle_panel(PANEL_R40); });
}

// -----------------------------------------------------------------------
MemView::~MemView() {}

// -----------------------------------------------------------------------
void MemView::connect_emu(EmuModel *emu)
{
	e = emu;
	connect(e, &EmuModel::signal_state_changed, this, &MemView::slot_state_changed);
	connect(e, &EmuModel::signal_reg_changed, this, &MemView::slot_reg_changed);
}

// -----------------------------------------------------------------------
void MemView::slot_state_changed(int state)
{
	// The displayed block is the user's choice; we don't auto-follow the CPU's
	// current NB. Editing live memory while the CPU runs would be racy, so we
	// only abandon an in-progress edit on entering RUN.
	cpu_running = (state == EM400_STATE_RUN);
	if (cpu_running) {
		cancel_edit();
	}
	update();
}

// -----------------------------------------------------------------------
void MemView::slot_reg_changed(int, uint16_t)
{
	// Repaint so the selected block shows live values as memory changes.
	update();
}

// -----------------------------------------------------------------------
int MemView::val_chars() const
{
	switch (fmt) {
		case FMT_HEX: return 5; // "xxxx "
		case FMT_UDEC: return 6; // "ddddd "
		case FMT_SDEC: return 7; // "-ddddd "
		case FMT_OFF: return 0;
	}
	return 5;
}

// -----------------------------------------------------------------------
int MemView::panel_chars() const
{
	switch (panel) {
		case PANEL_OFF: return 0;
		case PANEL_ASCII: return 2;
		case PANEL_R40: return 3;
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
	if (chars_per_word < 1) return 1;
	int max_wpl = avail / (chars_per_word * font_width);
	if (max_wpl < 1) return 1;
	// Align the line stride to a round hex boundary so addresses stay legible:
	// a multiple of 0x10 once that many words fit, otherwise the largest power
	// of two that does (degrading down to 1 on a view too narrow for 16).
	if (max_wpl >= 16) return (max_wpl / 16) * 16;
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
	// clicking the active format toggles the numeric column off; refuse if that
	// would leave nothing visible (never let the view go fully blank)
	DisplayFormat nf = (fmt == f) ? FMT_OFF : f;
	if (nf == FMT_OFF && panel == PANEL_OFF) {
		// a checkable button already flipped its own checked state on the click;
		// restore it so the refused toggle leaves no visible trace
		btn_hex->setChecked(fmt == FMT_HEX);
		btn_udec->setChecked(fmt == FMT_UDEC);
		btn_sdec->setChecked(fmt == FMT_SDEC);
		return;
	}
	if (fmt == nf) return;
	cancel_edit();
	fmt = nf;
	btn_hex->setChecked(fmt == FMT_HEX);
	btn_udec->setChecked(fmt == FMT_UDEC);
	btn_sdec->setChecked(fmt == FMT_SDEC);
	apply_wpl_change();
	update();
}

// -----------------------------------------------------------------------
void MemView::toggle_panel(SidePanel p)
{
	// same invariant as set_format: refuse to turn off the last visible column
	SidePanel np = (panel == p) ? PANEL_OFF : p;
	if (np == PANEL_OFF && fmt == FMT_OFF) {
		// restore the button's checked state, flipped by the click itself
		btn_ascii->setChecked(panel == PANEL_ASCII);
		btn_r40->setChecked(panel == PANEL_R40);
		return;
	}
	cancel_edit();
	panel = np;
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
	// ceil: a words_per_line that doesn't divide 0x10000 (e.g. 48) leaves a
	// partial final line whose valid words would otherwise only ever render in
	// the clipped bottom strip, unreachable at max scroll
	const int mem_lines = (0x10000 + words_per_line - 1) / words_per_line;
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
	if (fmt == FMT_OFF) return false;
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
// Map a widget-space point to an absolute address plus sub-char index in
// the side panel. Returns false outside the populated panel cells.
bool MemView::hit_test_panel(const QPoint &pos, int &addr, int &sub) const
{
	if (panel == PANEL_OFF) return false;

	int cell_w = val_chars() * font_width;
	int pcell_w = panel_chars() * font_width;
	int side_x = mem_x_start + words_per_line * cell_w + font_width;

	int cy = pos.y() - content_top - col_hdr_h;
	if (cy < 0) return false;
	int row = cy / line_height;
	if (row >= total_lines) return false;

	int cx = pos.x() - side_x;
	if (cx < 0) return false;
	int col = cx / pcell_w;
	if (col >= words_per_line) return false;

	int s = (cx - col * pcell_w) / font_width;
	if (s >= panel_chars()) s = panel_chars() - 1;

	int a = caddr + row * words_per_line + col;
	if (a > 0xffff) return false;

	addr = a;
	sub = s;
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
		case FMT_OFF:
			return;
	}

	editing = true;
	edit_kind = EDIT_VALUE;
	edit_nb = cnb;
	edit_addr = addr;
	edit_cursor = 0;
	setFocus();
	emit signal_edit_mode_changed(true, edit_insert);
	update();
}

// -----------------------------------------------------------------------
// Begin editing the side panel as a character stream. sub is the initial
// sub-char caret within the word (0..panel_chars()-1).
void MemView::start_text_edit(int addr, int sub)
{
	if (cpu_running || !e || panel == PANEL_OFF) return;

	// nothing to edit where no memory is mapped (reads back as -1)
	if (e->get_mem(cnb, addr) < 0) return;

	editing = true;
	edit_kind = EDIT_TEXT;
	edit_nb = cnb;
	edit_addr = addr;
	edit_char = sub;
	edit_orig.clear();
	setFocus();
	// text editing is always overwrite; report it as such
	emit signal_edit_mode_changed(true, false);
	update();
}

// -----------------------------------------------------------------------
// Overwrite the active sub-char of the word under edit and advance. For
// ASCII the byte is spliced directly; for R40 the word is decoded, the one
// char replaced, and re-encoded (the untouched chars round-trip losslessly).
void MemView::text_write_char(QChar c)
{
	if (!e) return;
	int val = e->get_mem(edit_nb, edit_addr);
	if (val < 0) return;
	uint16_t word = (uint16_t)val;

	if (panel == PANEL_ASCII) {
		unsigned char ch = (unsigned char)c.toLatin1();
		if (edit_char == 0) {
			word = (word & 0x00ff) | (ch << 8);
		} else {
			word = (word & 0xff00) | ch;
		}
	} else {
		// Replace only the one base-40 digit. Decoding the whole word to
		// ASCII and re-encoding would truncate at an embedded code-0 char
		// (which decodes to a NUL string terminator), corrupting the rest.
		// Get the typed char's code via a single-char encode, then splice.
		char tmp[2] = { (char)c.toLatin1(), 0 };
		unsigned len = 0;
		uint16_t enc = 0;
		if (!ascii_to_r40(tmp, &len, &enc)) return;
		static const int place[3] = { 1600, 40, 1 };
		int p = place[edit_char];
		int code = enc / 1600;
		int old = (word / p) % 40;
		word = word - old * p + code * p;
	}

	// snapshot the word's pre-edit value once, so the whole session can be
	// rolled back on cancel
	if (!edit_orig.contains(edit_addr)) {
		edit_orig.insert(edit_addr, (uint16_t)val);
	}
	e->set_mem(edit_nb, edit_addr, word);
	text_move(1);
}

// -----------------------------------------------------------------------
// Move the text caret by delta sub-chars, rolling across word boundaries
// and clamping to the address space, then scroll to keep it on screen.
void MemView::text_move(int delta)
{
	int cols = panel_chars();
	long pos = (long)edit_addr * cols + edit_char + delta;
	if (pos < 0) pos = 0;
	long maxpos = (long)0xffff * cols + (cols - 1);
	if (pos > maxpos) pos = maxpos;
	edit_addr = pos / cols;
	edit_char = pos % cols;
	ensure_caret_visible();
	update();
}

// -----------------------------------------------------------------------
// Scroll the view so the line holding the text caret is visible. Bypasses
// the scrollbar's valueChanged slot (which would cancel the edit) and moves
// caddr directly.
void MemView::ensure_caret_visible()
{
	// total_lines counts a partially-clipped bottom row; scroll against the
	// fully-visible count so the caret never lands in the clipped strip
	int visible = (bottom - col_hdr_h) / line_height;
	if (visible < 1) visible = 1;

	int line = edit_addr / words_per_line;
	int top_line = caddr / words_per_line;
	int target = top_line;
	if (line < top_line) {
		target = line;
	} else if (line >= top_line + visible) {
		target = line - visible + 1;
	}
	if (target == top_line) return;

	QSignalBlocker blk(scroll);
	scroll->setValue(target);
	caddr = scroll->value() * words_per_line;
}

// -----------------------------------------------------------------------
void MemView::cancel_edit()
{
	if (!editing) return;

	// text edits are write-through; "cancel" rolls every touched word back to
	// its pre-edit value, matching the value editor's ESC=discard semantics.
	// FIXME: these rollback writes also fire on the RUN transition (via
	// slot_state_changed -> cancel_edit), racing the CPU thread. Currently
	// mitigated because clicking away to start the CPU triggers focusOut ->
	// cancel_edit while still stopped, but that ordering is fragile; consider
	// suppressing the rollback (or only snapshotting) while cpu_running.
	if (edit_kind == EDIT_TEXT && e) {
		for (auto it = edit_orig.constBegin() ; it != edit_orig.constEnd() ; ++it) {
			e->set_mem(edit_nb, it.key(), it.value());
		}
	}
	edit_orig.clear();

	editing = false;
	edit_buf.clear();
	emit signal_edit_mode_changed(false, edit_insert);
	update();
}

// -----------------------------------------------------------------------
void MemView::commit_edit()
{
	if (!editing) return;

	// text edits are already written through; commit just keeps them and ends
	if (edit_kind == EDIT_TEXT) {
		edit_orig.clear();
		editing = false;
		emit signal_edit_mode_changed(false, edit_insert);
		update();
		return;
	}

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
			case FMT_OFF:
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
		case FMT_OFF:
			return false;
	}
	return false;
}

// -----------------------------------------------------------------------
void MemView::mouseDoubleClickEvent(QMouseEvent *event)
{
	int addr, sub;
	if (event->button() == Qt::LeftButton) {
		if (hit_test_cell(event->pos(), addr)) {
			start_edit(addr);
			event->accept();
			return;
		}
		if (hit_test_panel(event->pos(), addr, sub)) {
			start_text_edit(addr, sub);
			event->accept();
			return;
		}
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
			case FMT_HEX: return "----";
			case FMT_UDEC: return "-----";
			case FMT_SDEC: return "------";
			case FMT_OFF: return QString();
		}
		return "----";
	}
	switch (fmt) {
		case FMT_HEX: return QString("%1").arg((uint16_t)val, 4, 16, QLatin1Char('0'));
		case FMT_UDEC: return QString("%1").arg((uint16_t)val, 5, 10, QLatin1Char(' '));
		case FMT_SDEC: return QString("%1").arg((int16_t)val, 6, 10, QLatin1Char(' '));
		case FMT_OFF: return QString();
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
	if (!r40_to_ascii(&word, 1, buf)) return "???";
	// code 0 decodes to a NUL byte, which has no glyph and would collapse the
	// monospace cell, mis-aligning the caret and click hit-testing. It is R40's
	// blank slot, so render it as a space.
	for (int i = 0 ; i < 3 ; i++) {
		if (buf[i] == '\0') buf[i] = ' ';
	}
	return QString::fromLatin1(buf, 3);
}

// -----------------------------------------------------------------------
// The non-scrolling row of column offsets above the memory dump.
void MemView::draw_offset_row(QPainter &painter, int cell_w, int pcell_w, int side_x)
{
	font.setBold(true);
	painter.setFont(font);
	painter.setPen(palette().color(QPalette::Text));
	// value column offsets
	if (fmt != FMT_OFF) {
		for (int x=0 ; x<words_per_line ; x++) {
			QString off_str = QString("%1").arg(x, val_chars() - 1, 16, QLatin1Char(' '));
			painter.drawText(mem_x_start + x * cell_w, font_height, off_str);
		}
	}
	// side panel offsets: the value and panel cells have different widths, so
	// the value offsets above don't line up over the panel - it gets its own
	// ticks. A 2-char ASCII cell can't caption every column without the offsets
	// running together, so label every other one; the wider 3-char R40 cell has
	// room for all.
	if (panel != PANEL_OFF) {
		int step = (panel == PANEL_R40) ? 1 : 2;
		for (int x=0 ; x<words_per_line ; x+=step) {
			painter.drawText(side_x + x * pcell_w, font_height, QString::number(x, 16));
		}
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

		if (fmt != FMT_OFF) {
			// a value-column edit overlays its cell and suppresses the side panel
			if (editing && edit_kind == EDIT_VALUE && cnb == edit_nb && addr == edit_addr) {
				draw_edit_cell(painter, x, y, cell_w);
				continue;
			}

			draw_value_cell(painter, x, y, val, cell_w);

			// companion outline: while text-editing, frame the same word in the
			// value column (green = same locus, mirrored; outline not fill so it
			// reads as passive next to the active edit cell)
			if (editing && edit_kind == EDIT_TEXT && cnb == edit_nb && addr == edit_addr) {
				int ex = mem_x_start + x * cell_w;
				int ey = col_hdr_h + y * line_height;
				int val_w = (val_chars() - 1) * font_width;
				painter.setPen(QPen(palette().color(QPalette::Highlight), 1));
				painter.setRenderHint(QPainter::Antialiasing, true);
				// half-pixel offset so the 1px stroke sits on pixel centers: keeps
				// the straight edges crisp, antialiasing only the rounded corners
				QRectF r(ex - half_font_width + 0.5, ey + 0.5, val_w + font_width - 1, line_height - 1);
				painter.drawRoundedRect(r, 2, 2);
				painter.setRenderHint(QPainter::Antialiasing, false);
			}
		}

		if (panel != PANEL_OFF) {
			if (editing && edit_kind == EDIT_TEXT && cnb == edit_nb && addr == edit_addr) {
				draw_panel_edit_cell(painter, x, y, val, pcell_w, side_x);
			} else if (editing && edit_kind == EDIT_TEXT && cnb == edit_nb && edit_orig.contains(addr)) {
				draw_panel_cell_edited(painter, x, y, addr, val, pcell_w, side_x);
			} else {
				draw_panel_cell(painter, x, y, val, pcell_w, side_x);
			}
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
// A side panel cell for a word edited earlier this session: sub-chars that
// differ from the pre-edit value "pop" in normal Text colour while the rest
// stay dim, so the user's changes stay visible after the caret moves on.
void MemView::draw_panel_cell_edited(QPainter &painter, int x, int y, int addr, int val, int pcell_w, int side_x)
{
	QString cur = panel_text(val);
	QString orig = panel_text((int)edit_orig.value(addr));
	// changed sub-chars pop in the green accent ("you are here" = the edit);
	// untouched ones stay dim
	QColor dim = em400_dim_text_color(palette());
	QColor accent = palette().color(QPalette::Highlight);
	int bx = side_x + x * pcell_w;
	int by = mem_y_start + y * line_height;

	for (int i = 0 ; i < cur.length() ; i++) {
		bool changed = (i >= orig.length()) || (cur.at(i) != orig.at(i));
		painter.setPen(changed ? accent : dim);
		painter.drawText(bx + i * font_width, by, QString(cur.at(i)));
	}
}

// -----------------------------------------------------------------------
// The side panel cell under text edit: the live ASCII/R40 rendering with
// the whole field highlighted and a caret block over the active sub-char.
void MemView::draw_panel_edit_cell(QPainter &painter, int x, int y, int val, int pcell_w, int side_x)
{
	QString text = panel_text(val);
	int ex = side_x + x * pcell_w;
	int ey = col_hdr_h + y * line_height;
	int baseline = mem_y_start + y * line_height;

	// 1px of slack each side so the caret block never ends flush on the edge
	painter.fillRect(ex - 1, ey, pcell_w + 2, line_height, palette().color(QPalette::Highlight));
	painter.setPen(palette().color(QPalette::HighlightedText));
	painter.drawText(ex, baseline, text);

	// overwrite caret: solid block over the active char, redrawn inverted
	int cur_x = ex + edit_char * font_width;
	painter.fillRect(cur_x, ey + 1, font_width, line_height - 2,
		palette().color(QPalette::HighlightedText));
	if (edit_char < text.length()) {
		painter.setPen(palette().color(QPalette::Highlight));
		painter.drawText(cur_x, baseline, QString(text.at(edit_char)));
	}
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

		draw_offset_row(painter, cell_w, pcell_w, side_x);

		for (int y = 0; y < total_lines; y++) {
			int base_addr = caddr + y * words_per_line;
			if (base_addr > 0xffff) break;
			draw_line(painter, y, base_addr, cell_w, pcell_w, side_x);
		}

		// divider between values and side panel (only when both are present)
		if (panel != PANEL_OFF && fmt != FMT_OFF) {
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
	if (editing && edit_kind == EDIT_TEXT) {
		key_text_edit(event);
		return;
	}
	if (editing) {
		key_value_edit(event);
		return;
	}
	key_navigate(event);
}

// -----------------------------------------------------------------------
// Side panel character-stream editing: caret movement and write-through of
// printable ASCII / valid R40 characters.
void MemView::key_text_edit(QKeyEvent *event)
{
	switch (event->key()) {
		case Qt::Key_Return:
		case Qt::Key_Enter:
			commit_edit();
			break;
		case Qt::Key_Escape:
			cancel_edit();
			break;
		case Qt::Key_Left:
		case Qt::Key_Backspace:
			// non-destructive: backspace just walks the caret back
			text_move(-1);
			break;
		case Qt::Key_Right:
			text_move(1);
			break;
		case Qt::Key_Up:
			text_move(-words_per_line * panel_chars());
			break;
		case Qt::Key_Down:
			text_move(words_per_line * panel_chars());
			break;
		default: {
			QString t = event->text();
			if (!t.isEmpty()) {
				QChar c = t.at(0);
				if (panel == PANEL_ASCII) {
					char ch = c.toLatin1();
					if (ch >= 0x20 && ch < 0x7f) {
						text_write_char(c);
					}
				} else {
					// R40 stores uppercase only; map then validate. Only
					// genuine R40 characters are accepted (space and code 0
					// are not R40 characters, so they cannot be entered).
					// Non-Latin1 keys (e.g. Polish diacritics) map to 0 via
					// toLatin1(), and r40_valid_char(0) is true (code 0), so
					// reject the 0 byte explicitly to keep them out.
					QChar uc = c.toUpper();
					char l = uc.toLatin1();
					if (l != 0 && r40_valid_char(l)) {
						text_write_char(uc);
					}
				}
			}
			break;
		}
	}
	event->accept();
}

// -----------------------------------------------------------------------
// Numeric value-column editing: a one-word overwrite/insert line editor.
void MemView::key_value_edit(QKeyEvent *event)
{
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
}

// -----------------------------------------------------------------------
// Not editing: format/panel toggles and scroll navigation.
void MemView::key_navigate(QKeyEvent *event)
{
	switch (event->key()) {
		case Qt::Key_H: set_format(FMT_HEX); break;
		case Qt::Key_U: set_format(FMT_UDEC); break;
		case Qt::Key_D: set_format(FMT_SDEC); break;
		case Qt::Key_A: toggle_panel(PANEL_ASCII); break;
		case Qt::Key_R: toggle_panel(PANEL_R40); break;
		case Qt::Key_PageUp: scroll->setValue(scroll->value() - total_lines); break;
		case Qt::Key_PageDown: scroll->setValue(scroll->value() + total_lines); break;
		case Qt::Key_Home: scroll->setValue(0); break;
		case Qt::Key_End: scroll->setValue(scroll->maximum()); break;
		default: QWidget::keyPressEvent(event); break;
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

// vim: tabstop=4 shiftwidth=4 autoindent

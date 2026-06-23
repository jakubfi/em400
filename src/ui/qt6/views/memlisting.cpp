//  Copyright (c) 2026 Jakub Filipowicz <jakubf@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include <cctype>
#include <cstring>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QWheelEvent>
#include <emcrk/r40.h>
#include "memlisting.h"
#include "memsearch.h"
#include "libem400.h"
#include "theme.h"

// -----------------------------------------------------------------------
MemListing::MemListing(QWidget *parent) : QWidget(parent)
{
	set_font("Monospace", 12);

	setFocusPolicy(Qt::WheelFocus);

	scroll = new QScrollBar(Qt::Vertical, this);
	scroll->setMinimum(0);
	scroll->setSingleStep(1);
	scroll->setVisible(false);
	update_scroll_range();

	connect(scroll, &QScrollBar::valueChanged, this, &MemListing::update_contents_no_nb);
}

// -----------------------------------------------------------------------
MemListing::~MemListing() {}

// -----------------------------------------------------------------------
void MemListing::set_nb(int nb)
{
	cancel_edit();
	cnb = nb;
	search_origin = -1; // the cursor was scoped to the old segment
	update();
}

// -----------------------------------------------------------------------
void MemListing::connect_emu(EmuModel *emu)
{
	e = emu;
	connect(e, &EmuModel::signal_state_changed, this, &MemListing::slot_state_changed);
	connect(e, &EmuModel::signal_reg_changed, this, &MemListing::slot_reg_changed);
}

// -----------------------------------------------------------------------
void MemListing::slot_state_changed(int state)
{
	// The displayed segment is the user's choice; we don't auto-follow the CPU's
	// current NB. Editing live memory while the CPU runs would be racy, so we
	// only abandon an in-progress edit on entering RUN.
	cpu_running = (state == EM400_STATE_RUN);
	if (cpu_running) {
		cancel_edit();
	}
	update();
}

// -----------------------------------------------------------------------
void MemListing::slot_reg_changed(int, uint16_t)
{
	// Repaint so the selected segment shows live values as memory changes.
	update();
}

// -----------------------------------------------------------------------
int MemListing::val_chars() const
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
int MemListing::panel_chars() const
{
	switch (panel) {
		case PANEL_OFF: return 0;
		case PANEL_ASCII: return 2;
		case PANEL_R40: return 3;
	}
	return 0;
}

// -----------------------------------------------------------------------
int MemListing::compute_words_per_line() const
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
QSize MemListing::sizeHint() const
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
	int h = col_hdr_h + pref_lines * line_height;
	return QSize(w, h);
}

// -----------------------------------------------------------------------
void MemListing::apply_wpl_change()
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
void MemListing::set_format(DisplayFormat f)
{
	// clicking the active format toggles the numeric column off; refuse if that
	// would leave nothing visible (never let the view go fully blank)
	DisplayFormat nf = (fmt == f) ? FMT_OFF : f;
	if (nf == FMT_OFF && panel == PANEL_OFF) {
		emit format_changed(fmt); // refuse: restore the controls to the live format
		return;
	}
	if (fmt == nf) return;
	cancel_edit();
	fmt = nf;
	emit format_changed(fmt);
	apply_wpl_change();
	update();
}

// -----------------------------------------------------------------------
void MemListing::toggle_panel(SidePanel p)
{
	// same invariant as set_format: refuse to turn off the last visible column
	SidePanel np = (panel == p) ? PANEL_OFF : p;
	if (np == PANEL_OFF && fmt == FMT_OFF) {
		emit panel_changed(panel);
		return;
	}
	cancel_edit();
	panel = np;
	emit panel_changed(panel);
	apply_wpl_change();
	update();
}

// -----------------------------------------------------------------------
void MemListing::set_font(QString name, int size)
{
	font.setFamily(name);
	if (size > 0) font.setPixelSize(size);
	setFont(font);
	font_bold = font;
	font_bold.setBold(true);
	QFontMetrics fm(font);
	font_height = fm.lineSpacing();
	font_width = fm.horizontalAdvance('9');
	update_font_related_dimensions();
}

// -----------------------------------------------------------------------
void MemListing::update_font_related_dimensions()
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
void MemListing::update_scroll_range()
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
void MemListing::update_contents(int new_nb, int new_addr)
{
	cnb = new_nb;
	caddr = (new_addr / words_per_line) * words_per_line;
	emit nb_changed(cnb);
	QSignalBlocker blk2(scroll);
	scroll->setValue(caddr / words_per_line);
	update();
}

// -----------------------------------------------------------------------
void MemListing::update_contents_no_nb(int new_line)
{
	int new_addr = new_line * words_per_line;
	if (new_addr == caddr) return;
	cancel_edit();
	caddr = new_addr;
	update();
}

// -----------------------------------------------------------------------
// Jump to a segment+address (placing its line at the top) and frame that cell
// with a green accent box. Driven by the disassembly view's "Locate in Memory
// View" context action.
void MemListing::locate_cell(int nb, int addr)
{
	cancel_edit();
	cnb = nb;

	// only scroll when the target line isn't already on screen; if it is, leave
	// the view put so the accent box just appears where it already sits. Use the
	// fully-visible line count (total_lines counts a clipped bottom row).
	int visible = (bottom - col_hdr_h) / line_height;
	if (visible < 1) visible = 1;
	int line = addr / words_per_line;
	int top_line = caddr / words_per_line;
	if (line < top_line || line >= top_line + visible) {
		caddr = line * words_per_line;
	}

	sel_nb = nb;
	sel_anchor = sel_caret = addr;
	emit nb_changed(cnb);
	QSignalBlocker blk2(scroll);
	scroll->setValue(caddr / words_per_line);
	update();
}

// -----------------------------------------------------------------------
// Map a widget-space point to an absolute address in the value column.
// Returns false for clicks on the header, address gutter, side panel or
// outside the populated cells.
bool MemListing::hit_test_cell(const QPoint &pos, int &addr) const
{
	if (fmt == FMT_OFF) return false;
	int cell_w = val_chars() * font_width;

	int cy = pos.y() - col_hdr_h;
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
bool MemListing::hit_test_panel(const QPoint &pos, int &addr, int &sub) const
{
	if (panel == PANEL_OFF) return false;

	int cell_w = val_chars() * font_width;
	int pcell_w = panel_chars() * font_width;
	int side_x = mem_x_start + words_per_line * cell_w + font_width;

	int cy = pos.y() - col_hdr_h;
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
void MemListing::start_edit(int addr)
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
	clear_selection();
	setFocus();
	emit signal_edit_mode_changed(true, edit_insert);
	update();
}

// -----------------------------------------------------------------------
// Begin editing the side panel as a character stream. sub is the initial
// sub-char caret within the word (0..panel_chars()-1).
void MemListing::start_text_edit(int addr, int sub)
{
	if (cpu_running || !e || panel == PANEL_OFF) return;

	// nothing to edit where no memory is mapped (reads back as -1)
	int val = e->get_mem(cnb, addr);
	if (val < 0) return;

	// R40 fills left-to-right, so the caret can't sit past the first blank
	// slot - snap it there so it matches where typing actually lands.
	if (panel == PANEL_R40) {
		static const int place[3] = { 1600, 40, 1 };
		int glyphs = 0;
		while (glyphs < 3 && ((uint16_t)val / place[glyphs]) % 40 != 0) glyphs++;
		if (sub > glyphs) sub = glyphs;
	}

	editing = true;
	edit_kind = EDIT_TEXT;
	edit_nb = cnb;
	edit_addr = addr;
	edit_char = sub;
	edit_orig.clear();
	clear_selection();
	setFocus();
	// text editing is always overwrite; report it as such
	emit signal_edit_mode_changed(true, false);
	update();
}

// -----------------------------------------------------------------------
// Overwrite the active sub-char of the word under edit and advance. For
// ASCII the byte is spliced directly; for R40 the word is decoded, the one
// char replaced, and re-encoded (the untouched chars round-trip losslessly).
void MemListing::text_write_char(QChar c)
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
		// R40 strings fill left-to-right; a glyph after a blank slot is not a
		// valid string. Clamp the write to the first blank slot so typing into
		// an empty/short word lands contiguously instead of leaving a hole.
		int glyphs = 0;
		while (glyphs < 3 && (word / place[glyphs]) % 40 != 0) glyphs++;
		if (edit_char > glyphs) edit_char = glyphs;
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
void MemListing::text_move(int delta)
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
// Scroll the view so the line holding addr is visible. Bypasses the
// scrollbar's valueChanged slot (which would cancel an edit) and moves caddr
// directly.
void MemListing::ensure_addr_visible(int addr)
{
	// total_lines counts a partially-clipped bottom row; scroll against the
	// fully-visible count so the line never lands in the clipped strip
	int visible = (bottom - col_hdr_h) / line_height;
	if (visible < 1) visible = 1;

	int line = addr / words_per_line;
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
void MemListing::ensure_caret_visible()
{
	ensure_addr_visible(edit_addr);
}

// -----------------------------------------------------------------------
void MemListing::cancel_edit()
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
void MemListing::commit_edit()
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
bool MemListing::valid_buf(const QString &s) const
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
// Left click selects the clicked cell with the green accent box; clicking the
// already-selected single cell de-selects it. Shift+click extends the existing
// selection from its anchor to the clicked cell. A press then begins a drag
// (see mouseMoveEvent) that grows the range. Works in the value column and the
// side panel.
void MemListing::mousePressEvent(QMouseEvent *event)
{
	// don't hijack clicks while a cell is being edited - let the edit run
	int addr, sub;
	if (!editing && event->button() == Qt::LeftButton
			&& (hit_test_cell(event->pos(), addr) || hit_test_panel(event->pos(), addr, sub))) {
		bool shift = event->modifiers() & Qt::ShiftModifier;
		if (shift && has_selection() && sel_nb == cnb) {
			sel_caret = addr; // extend from the fixed anchor
		} else if (!shift && has_selection() && sel_nb == cnb
				&& sel_anchor == addr && sel_caret == addr) {
			clear_selection(); // toggle off a single-cell selection
		} else {
			sel_nb = cnb;
			sel_anchor = sel_caret = addr;
		}
		update();
		event->accept();
		return;
	}
	QWidget::mousePressEvent(event);
}

// -----------------------------------------------------------------------
// Dragging with the left button held grows the selection: the anchor stays
// put and the cell under the cursor becomes the moving end of the range.
void MemListing::mouseMoveEvent(QMouseEvent *event)
{
	int addr, sub;
	if (!editing && (event->buttons() & Qt::LeftButton)
			&& has_selection() && sel_nb == cnb
			&& (hit_test_cell(event->pos(), addr) || hit_test_panel(event->pos(), addr, sub))) {
		if (addr != sel_caret) {
			sel_caret = addr;
			update();
		}
		event->accept();
		return;
	}
	QWidget::mouseMoveEvent(event);
}

// -----------------------------------------------------------------------
void MemListing::mouseDoubleClickEvent(QMouseEvent *event)
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
void MemListing::focusOutEvent(QFocusEvent *event)
{
	cancel_edit();
	QWidget::focusOutEvent(event);
}

// -----------------------------------------------------------------------
// Run the search from the cursor in the given direction (via MemSearch) and, on
// a hit, follow the view to it: switch to the hit's segment, reveal the matching
// side pane, frame the matched word run with the green selection box (the hit IS
// the current locus) and remember it as the cursor NEXT / PREV resumes from. The
// dock supplies the query and renders the outcome we return as a cue.
MemListing::SearchOutcome MemListing::search(const QString &query, MemSearch::Mode mode, bool all_segments, bool forward)
{
	if (!e) return SEARCH_NONE;
	if (query.trimmed().isEmpty() || !MemSearch::query_valid(query, mode)) return SEARCH_NONE;

	MemSearch search(e);
	MemSearch::Result r;
	if (!search.find(query, mode, all_segments, cnb, search_origin, forward, r)) {
		return SEARCH_MISS;
	}

	// the view follows the match: switch to its segment if it landed elsewhere
	if (r.nb != cnb) {
		cancel_edit();
		cnb = r.nb;
		emit nb_changed(cnb);
	}
	// a stream search matched characters, so reveal the matching side pane (the
	// hit boxes whole cells, but the chars that matched only read in that pane).
	// toggle_panel toggles, so only call it when the pane isn't already showing.
	SidePanel want = (mode == MemSearch::ASCII) ? PANEL_ASCII : (mode == MemSearch::R40) ? PANEL_R40 : panel;
	if (want != panel) toggle_panel(want);
	search_origin = r.found;
	sel_nb = cnb;
	sel_anchor = r.found;
	sel_caret = r.last;
	ensure_addr_visible(r.found);
	update();
	return r.wrapped ? SEARCH_WRAPPED : SEARCH_FOUND;
}

// -----------------------------------------------------------------------
// Format the value column text for one word. val < 0 means the word is
// unreadable (no memory mapped), shown as dashes.
QString MemListing::value_text(int val) const
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
QString MemListing::panel_text(int val) const
{
	if (panel == PANEL_ASCII) {
		if (val < 0) return "..";
		unsigned char hi = (val >> 8) & 0xff;
		unsigned char lo = val & 0xff;
		return QString(QChar(isprint(hi) ? hi : '.')) + QString(QChar(isprint(lo) ? lo : '.'));
	}
	// R40
	if (val < 0) return "???";
	uint16_t word = (uint16_t)val;
	char buf[4];
	// A word that isn't a valid R40 string (e.g. a glyph after a blank slot)
	// is no more a string than an all-blank word, so render both as blanks
	// rather than cluttering the column with "???".
	if (!r40_to_ascii(&word, 1, buf)) return "   ";
	// r40_to_ascii now stops at the first blank slot (code 0) and terminates,
	// so the result is 0..3 chars. Right-pad with spaces to keep the monospace
	// cell width fixed for caret placement and click hit-testing.
	for (size_t i = strlen(buf) ; i < 3 ; i++) {
		buf[i] = ' ';
	}
	return QString::fromLatin1(buf, 3);
}

// -----------------------------------------------------------------------
void MemListing::draw_offset_row(QPainter &painter, int cell_w, int pcell_w, int side_x)
{
	painter.setPen(QPen(em400_sep_color(palette()), 2));
	painter.drawLine(divider_x_pos, col_hdr_h, right - 1, col_hdr_h);

	// no contents if not powered on
	if (!e || !e->is_powered()) {
		return;
	}

	painter.setFont(font_bold);
	painter.setPen(palette().color(QPalette::Text));

	// value column offsets
	if (fmt != FMT_OFF) {
		for (int x=0 ; x<words_per_line ; x++) {
			QString off_str = QString("%1").arg(x, val_chars() - 1, 16, QLatin1Char(' '));
			painter.drawText(mem_x_start + x * cell_w, font_height, off_str);
		}
	}

	// side panel offsets
	if (panel != PANEL_OFF) {
		int step = (panel == PANEL_R40) ? 1 : 2;
		for (int x=0 ; x<words_per_line ; x+=step) {
			painter.drawText(side_x + x * pcell_w, font_height, QString::number(x, 16));
		}
	}
}

// -----------------------------------------------------------------------
void MemListing::draw_line(QPainter &painter, int y, int base_addr, int cell_w, int pcell_w, int side_x)
{
	QString addr_str = QString("%1").arg((uint16_t)base_addr, 4, 16, QLatin1Char('0'));
	painter.setFont(font_bold);
	painter.setPen(palette().color(QPalette::Text));
	painter.drawText(addr_x_start, addr_y_start + y * line_height, addr_str);

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

	// drawn in a second pass, after every cell on the line: the box bleeds a bit
	// past its cell and would otherwise be painted over by the next cell's chars.
	// A selection spanning several lines yields one box per line, each covering
	// the contiguous run of selected columns that falls on that line.
	if (sel_nb == cnb && has_selection()) {
		int line_hi = base_addr + words_per_line - 1;
		if (line_hi > 0xffff) line_hi = 0xffff;
		int a = qMax(sel_lo(), base_addr);
		int b = qMin(sel_hi(), line_hi);
		if (a <= b) {
			draw_locate_box(painter, a - base_addr, b - base_addr, y, cell_w, pcell_w, side_x);
		}
	}
}

// -----------------------------------------------------------------------
// Frame the selected cells (columns col0..col1 inclusive on this line) with the
// green accent box used by "Locate in Memory View" - same look as the editor's
// companion outline. The numeric and the side-panel run are each boxed when
// their column is shown, so the selection stays framed in both halves at once.
void MemListing::draw_locate_box(QPainter &painter, int col0, int col1, int y, int cell_w, int pcell_w, int side_x)
{
	int ey = col_hdr_h + y * line_height;
	int ncells = col1 - col0 + 1;

	painter.setPen(QPen(palette().color(QPalette::Highlight), 1));
	painter.setRenderHint(QPainter::Antialiasing, true);

	// half-pixel offset so the 1px stroke sits on pixel centers (crisp edges).
	// Height is line_height (not -1) so the bottom edge of one row's box lands on
	// the top edge of the next: a multi-row selection reads as one continuous
	// box instead of two stacked 1px lines where rows touch.
	if (fmt != FMT_OFF) {
		int bx = mem_x_start + col0 * cell_w - half_font_width;
		QRectF r(bx + 0.5, ey + 0.5, ncells * cell_w - 1, line_height);
		painter.drawRoundedRect(r, 2, 2);
	}
	if (panel != PANEL_OFF) {
		// pull the panel box in by 2px each side: panel cells are packed with no
		// trailing pad, so the full half-font margin would bleed into neighbours
		int bx = side_x + col0 * pcell_w - half_font_width + 2;
		QRectF r(bx + 0.5, ey + 0.5, ncells * pcell_w + font_width - 5, line_height);
		painter.drawRoundedRect(r, 2, 2);
	}

	painter.setRenderHint(QPainter::Antialiasing, false);
}

// -----------------------------------------------------------------------
void MemListing::draw_value_cell(QPainter &painter, int x, int y, int val, int cell_w)
{
	painter.setPen(palette().color(QPalette::Text));
	painter.drawText(mem_x_start + x * cell_w, mem_y_start + y * line_height, value_text(val));
}

// -----------------------------------------------------------------------
void MemListing::draw_panel_cell(QPainter &painter, int x, int y, int val, int pcell_w, int side_x)
{
	painter.setPen(em400_dim_text_color(palette()));
	painter.drawText(side_x + x * pcell_w, mem_y_start + y * line_height, panel_text(val));
}

// -----------------------------------------------------------------------
// A side panel cell for a word edited earlier this session: sub-chars that
// differ from the pre-edit value "pop" in normal Text colour while the rest
// stay dim, so the user's changes stay visible after the caret moves on.
void MemListing::draw_panel_cell_edited(QPainter &painter, int x, int y, int addr, int val, int pcell_w, int side_x)
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
void MemListing::draw_panel_edit_cell(QPainter &painter, int x, int y, int val, int pcell_w, int side_x)
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
void MemListing::draw_edit_cell(QPainter &painter, int x, int y, int cell_w)
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
void MemListing::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.setClipRect(0, 0, right + 1, bottom + 1);

	painter.fillRect(0, 0, right, bottom, palette().color(QPalette::Base));

	// divider between addr and values
	painter.setPen(QPen(em400_sep_color(palette()), 2));
	painter.drawLine(divider_x_pos, 0, divider_x_pos, bottom);

	int cell_w = val_chars() * font_width;
	int pcell_w = panel_chars() * font_width;
	int side_x = (panel != PANEL_OFF)
		? mem_x_start + words_per_line * cell_w + font_width
		: 0;

	// divider between values and side panel (only when both are present)
	if (panel != PANEL_OFF && fmt != FMT_OFF) {
		int sdiv_x = mem_x_start + words_per_line * cell_w + half_font_width;
		painter.setPen(QPen(em400_sep_color(palette()), 2));
		painter.drawLine(sdiv_x, 0, sdiv_x, bottom);
	}

	draw_offset_row(painter, cell_w, pcell_w, side_x);

	// no contents if not powered on
	if (!e || !e->is_powered()) {
		return;
	}

	for (int y = 0; y < total_lines; y++) {
		int base_addr = caddr + y * words_per_line;
		if (base_addr > 0xffff) break;
		draw_line(painter, y, base_addr, cell_w, pcell_w, side_x);
	}
}

// -----------------------------------------------------------------------
void MemListing::relayout_grid()
{
	bottom = height();
	right = width();
	total_lines = (bottom - col_hdr_h) / line_height + 1;

	apply_wpl_change();
	scroll->setPageStep(total_lines);
	scroll->setGeometry(width() - scroll->sizeHint().width(), col_hdr_h,
		scroll->sizeHint().width(), bottom - col_hdr_h);
}

// -----------------------------------------------------------------------
void MemListing::resizeEvent(QResizeEvent *event)
{
	relayout_grid();
	QWidget::resizeEvent(event);
}

// -----------------------------------------------------------------------
int MemListing::calculate_scroll_lines(int angle_delta)
{
	const int native_wheel_step = 120;
	const int lines_per_click = 3;
	const int one_line_advance = native_wheel_step / lines_per_click;

	wheel_tick_accumulator += angle_delta;
	int lines = wheel_tick_accumulator / one_line_advance;
	wheel_tick_accumulator %= one_line_advance;

	return lines;
}

// -----------------------------------------------------------------------
void MemListing::wheelEvent(QWheelEvent *event)
{
	const int delta_lines = calculate_scroll_lines(event->angleDelta().y());
	scroll->setValue(scroll->value() - delta_lines);
	event->accept();
}

// -----------------------------------------------------------------------
void MemListing::keyPressEvent(QKeyEvent *event)
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
void MemListing::key_text_edit(QKeyEvent *event)
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
void MemListing::key_value_edit(QKeyEvent *event)
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
void MemListing::key_navigate(QKeyEvent *event)
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
void MemListing::enterEvent(QEnterEvent *event)
{
	scroll->setVisible(true);
	QWidget::enterEvent(event);
}

// -----------------------------------------------------------------------
void MemListing::leaveEvent(QEvent *event)
{
	scroll->setVisible(false);
	QWidget::leaveEvent(event);
}

// vim: tabstop=4 shiftwidth=4 autoindent

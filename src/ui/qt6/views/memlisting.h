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

#ifndef MEMLISTING_H
#define MEMLISTING_H

#include <QWidget>
#include <QFont>
#include <QHash>
#include "emumodel.h"
#include "memsearch.h"

// only used as pointers / by-pointer override params below; full definitions
// are pulled into the .cpp
class QPainter;
class QScrollBar;
class QKeyEvent;
class QMouseEvent;
class QFocusEvent;

// -----------------------------------------------------------------------
// The scrolling memory grid: a plain 0,0-relative painted box. Segment, display
// format, side panel and search are driven from outside (the dock owns those
// controls); the grid performs the work and signals back so the controls track.
class MemListing : public QWidget {

	Q_OBJECT

public:
	enum DisplayFormat { FMT_HEX, FMT_UDEC, FMT_SDEC, FMT_OFF };
	enum SidePanel { PANEL_OFF, PANEL_ASCII, PANEL_R40 };
	enum EditKind { EDIT_VALUE, EDIT_TEXT };
	enum SearchOutcome { SEARCH_NONE, SEARCH_FOUND, SEARCH_WRAPPED, SEARCH_MISS };

	explicit MemListing(QWidget *parent = nullptr);
	~MemListing();
	void connect_emu(EmuModel *emu);

	QSize sizeHint() const override;

	// Run a search from the current cursor and follow the view to a hit. The dock
	// supplies the query from its strip; the grid scans (via MemSearch), moves
	// itself to the match and reports the outcome so the dock can show the cue.
	SearchOutcome search(const QString &query, MemSearch::Mode mode, bool all_segments, bool forward);
	// the cursor resumes from the last hit; reset it when query / mode change
	void reset_search_origin() { search_origin = -1; }

public slots:
	void update_contents(int nb, int addr);
	// jump to a segment+address and frame that cell with a green accent box (same
	// accent the editor uses); the box stays until the cell is clicked again,
	// another cell is selected, or an edit begins
	void locate_cell(int nb, int addr);
	void set_nb(int nb);
	void set_format(DisplayFormat f);
	void toggle_panel(SidePanel p);
	void refresh_font();

private slots:
	void slot_state_changed(int state);
	void slot_reg_changed(int reg, uint16_t val);
	void update_contents_no_nb(int new_line);

signals:
	// editing==false means no cell is being edited; insert reflects the
	// current insert/overwrite mode while editing
	void signal_edit_mode_changed(bool editing, bool insert);
	// the grid changed what it shows on its own (a followed search hit / a refused
	// or toggled column); the dock's controls mirror these
	void nb_changed(int nb);
	void format_changed(DisplayFormat fmt);
	void panel_changed(SidePanel panel);

private:
	// geometry
	int total_lines = 0;
	// grid extents: bottom is the scrolling area's height, right is the full width
	int bottom = 100, right = 100;
	int half_font_width;
	int line_height;
	int col_hdr_h; // height of the non-scrolling column offset row (= line_height)
	int addr_x_start, addr_y_start;
	int addr_len;
	int mem_x_start, mem_y_start;
	int divider_x_pos;
	int words_per_line = 16;

	// view state
	int cnb = 0, caddr = 0;
	// Cell selection framed with the green accent box: a contiguous address
	// range [sel_anchor .. sel_caret] within segment sel_nb (either end may be the
	// larger; sel_lo()/sel_hi() order them). sel_anchor is the fixed origin a
	// shift-click or drag extends from; sel_caret is the moving end. Empty when
	// sel_anchor < 0. Drives "Locate in Memory View" (a single cell) and
	// click/shift-click/drag selection. Persists across scrolling; cleared when
	// a single-cell selection is clicked again, or when an edit begins. The box
	// only paints while its segment (sel_nb) is the one on screen.
	int sel_nb = -1, sel_anchor = -1, sel_caret = -1;
	bool has_selection() const { return sel_anchor >= 0; }
	void clear_selection() { sel_nb = -1; sel_anchor = sel_caret = -1; }
	int sel_lo() const { return sel_anchor < sel_caret ? sel_anchor : sel_caret; }
	int sel_hi() const { return sel_anchor > sel_caret ? sel_anchor : sel_caret; }
	DisplayFormat fmt = FMT_HEX;
	SidePanel panel = PANEL_ASCII;
	bool cpu_running = false;

	// in-place cell editing. EDIT_VALUE edits the numeric value column:
	// an overwrite-style line editor where edit_buf holds the digits and
	// edit_cursor is the caret column (0..edit_buf.length()), committed as
	// one word. EDIT_TEXT edits the ASCII/R40 side panel as a write-through
	// character stream: every keystroke overwrites one sub-char of the word
	// at edit_addr (edit_char selects which) and advances, rolling onto the
	// next word at the boundary so strings can be typed straight through.
	bool editing = false;
	EditKind edit_kind = EDIT_VALUE;
	int edit_nb = 0;
	int edit_addr = 0;
	QString edit_buf;
	int edit_cursor = 0;
	bool edit_insert = false; // false = overwrite, toggled with Insert
	int edit_char = 0; // EDIT_TEXT: sub-char caret within the word
	// EDIT_TEXT: original value of each word touched this session, so ESC /
	// focus-loss can roll the write-through edits back (addr -> pre-edit word)
	QHash<int, uint16_t> edit_orig;

	bool hit_test_cell(const QPoint &pos, int &addr) const;
	bool hit_test_panel(const QPoint &pos, int &addr, int &sub) const;
	void start_edit(int addr);
	void start_text_edit(int addr, int sub);
	void commit_edit();
	void cancel_edit();
	bool valid_buf(const QString &s) const;
	void text_write_char(QChar c);
	void text_move(int delta);
	void ensure_caret_visible();
	void ensure_addr_visible(int addr); // scroll so the line holding addr is on screen

	// keyPressEvent dispatches to one of these by current edit mode
	void key_text_edit(QKeyEvent *event);
	void key_value_edit(QKeyEvent *event);
	void key_navigate(QKeyEvent *event);

	int wheel_tick_accumulator = 0;

	QScrollBar *scroll = nullptr;

	// the search cursor: start word of the last match. NEXT resumes one word past
	// it, PREV one word before. -1 means search from the top (NEXT) / bottom (PREV).
	int search_origin = -1;

	EmuModel *e = nullptr;
	QFont font; // the regular grid font
	QFont font_bold; // bold variant for the address gutter and offset headers
	int font_height, font_width;

	int val_chars() const;
	int panel_chars() const;
	int compute_words_per_line() const;
	void apply_wpl_change();

	// paintEvent helpers
	void draw_offset_row(QPainter &painter, int cell_w, int pcell_w, int side_x);
	void draw_line(QPainter &painter, int y, int base_addr, int cell_w, int pcell_w, int side_x);
	void draw_value_cell(QPainter &painter, int x, int y, int val, int cell_w);
	void draw_edit_cell(QPainter &painter, int x, int y, int cell_w);
	void draw_panel_cell(QPainter &painter, int x, int y, int val, int pcell_w, int side_x);
	void draw_panel_cell_edited(QPainter &painter, int x, int y, int addr, int val, int pcell_w, int side_x);
	void draw_panel_edit_cell(QPainter &painter, int x, int y, int val, int pcell_w, int side_x);
	void draw_locate_box(QPainter &painter, int col0, int col1, int y, int cell_w, int pcell_w, int side_x);
	QString value_text(int val) const;
	QString panel_text(int val) const;

	void apply_font();
	void update_scroll_range();
	void relayout_grid();
	int calculate_scroll_lines(int angle_delta);

protected:
	void paintEvent(QPaintEvent *event) override;
	void resizeEvent(QResizeEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseDoubleClickEvent(QMouseEvent *event) override;
	void focusOutEvent(QFocusEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;
	void enterEvent(QEnterEvent *event) override;
	void leaveEvent(QEvent *event) override;
	void keyPressEvent(QKeyEvent *event) override;
};

#endif // MEMLISTING_H

// vim: tabstop=4 shiftwidth=4 autoindent

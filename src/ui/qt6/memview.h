#ifndef MEMVIEW_H
#define MEMVIEW_H

#include <QWidget>
#include <QScrollBar>
#include <QSpinBox>
#include <QPushButton>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QHash>
#include "emumodel.h"

class QPainter;

// -----------------------------------------------------------------------
class MemView : public QWidget {

	Q_OBJECT

public:
	enum DisplayFormat { FMT_HEX, FMT_UDEC, FMT_SDEC, FMT_OFF };
	enum SidePanel { PANEL_OFF, PANEL_ASCII, PANEL_R40 };
	enum EditKind { EDIT_VALUE, EDIT_TEXT };

	explicit MemView(QWidget *parent = nullptr);
	~MemView();
	void connect_emu(EmuModel *emu);

	QSize sizeHint() const override;

public slots:
	void update_contents(int nb, int addr);
	void update_contents_no_nb(int new_line);
	// jump to a segment+address and frame that cell with a green accent box (same
	// accent the editor uses); the box stays until the cell is clicked again,
	// another cell is selected, or an edit begins
	void locate_cell(int nb, int addr);

private slots:
	void slot_state_changed(int state);
	void slot_reg_changed(int reg, uint16_t val);

signals:
	// editing==false means no cell is being edited; insert reflects the
	// current insert/overwrite mode while editing
	void signal_edit_mode_changed(bool editing, bool insert);

private:
	// geometry
	int content_top = 0;
	int total_lines = 0;
	int bottom, right;
	int half_font_width;
	int line_height;
	int col_hdr_h;   // height of the non-scrolling column offset row (= line_height)
	int addr_x_start, addr_y_start;
	int addr_len;
	int mem_x_start, mem_y_start;
	int divider_x_pos;
	int words_per_line;

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

	// keyPressEvent dispatches to one of these by current edit mode
	void key_text_edit(QKeyEvent *event);
	void key_value_edit(QKeyEvent *event);
	void key_navigate(QKeyEvent *event);

	int wheel_tick_accumulator = 0;

	// child widgets
	QWidget *header;
	QSpinBox *nb_spin;
	QPushButton *btn_hex, *btn_udec, *btn_sdec;
	QPushButton *btn_ascii, *btn_r40;
	QScrollBar *scroll;

	EmuModel *e = nullptr;
	QFont font;
	int font_height, font_width;

	int val_chars() const;
	int panel_chars() const;
	int compute_words_per_line() const;
	void apply_wpl_change();
	void set_format(DisplayFormat f);
	void toggle_panel(SidePanel p);

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

	void set_font(QString name, int size = 0);
	void update_font_related_dimensions();
	void update_scroll_range();
	int calculate_scroll_lines(int angleDelta);

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

#endif // MEMVIEW_H

// vim: tabstop=4 shiftwidth=4 autoindent

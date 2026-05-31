#ifndef MEMVIEW_H
#define MEMVIEW_H

#include <QWidget>
#include <QScrollBar>
#include <QSpinBox>
#include <QPushButton>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFocusEvent>
#include "emumodel.h"

class QPainter;

// -----------------------------------------------------------------------
class MemView : public QWidget {

	Q_OBJECT

public:
	enum DisplayFormat { FMT_HEX, FMT_UDEC, FMT_SDEC };
	enum SidePanel { PANEL_OFF, PANEL_ASCII, PANEL_R40 };

	explicit MemView(QWidget *parent = nullptr);
	~MemView();
	void connect_emu(EmuModel *emu);

	QSize sizeHint() const override;

public slots:
	void update_contents(int nb, int addr);
	void update_contents_no_nb(int new_line);

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
	DisplayFormat fmt = FMT_HEX;
	SidePanel panel = PANEL_ASCII;
	bool cpu_running = false;

	// in-place cell editing (hex/dec/octal value column).
	// Overwrite-style line editor: edit_buf holds the digits, edit_cursor
	// is the caret column (0..edit_buf.length()).
	bool editing = false;
	int edit_nb = 0;
	int edit_addr = 0;
	QString edit_buf;
	int edit_cursor = 0;
	bool edit_insert = false; // false = overwrite, toggled with Insert

	bool hit_test_cell(const QPoint &pos, int &addr) const;
	void start_edit(int addr);
	void commit_edit();
	void cancel_edit();
	bool valid_buf(const QString &s) const;

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
	void draw_offset_row(QPainter &painter, int cell_w);
	void draw_line(QPainter &painter, int y, int base_addr, int cell_w, int pcell_w, int side_x);
	void draw_value_cell(QPainter &painter, int x, int y, int val, int cell_w);
	void draw_edit_cell(QPainter &painter, int x, int y, int cell_w);
	void draw_panel_cell(QPainter &painter, int x, int y, int val, int pcell_w, int side_x);
	QString value_text(int val) const;
	QString panel_text(int val) const;

	void set_font(QString name, int size = 0);
	void update_font_related_dimensions();
	void update_scroll_range();
	int calculate_scroll_lines(int angleDelta);

protected:
	void paintEvent(QPaintEvent *event) override;
	void resizeEvent(QResizeEvent *event) override;
	void mouseDoubleClickEvent(QMouseEvent *event) override;
	void focusOutEvent(QFocusEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;
	void enterEvent(QEnterEvent *event) override;
	void leaveEvent(QEvent *event) override;
	void keyPressEvent(QKeyEvent *event) override;
};

#endif // MEMVIEW_H

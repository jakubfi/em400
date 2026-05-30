#ifndef MEMVIEW_H
#define MEMVIEW_H

#include <QWidget>
#include <QScrollBar>
#include <QSpinBox>
#include <QPushButton>
#include <QKeyEvent>
#include "emumodel.h"

// -----------------------------------------------------------------------
class MemView : public QWidget {

	Q_OBJECT

public:
	enum DisplayFormat { FMT_HEX, FMT_UDEC, FMT_SDEC };
	enum SidePanel { PANEL_OFF, PANEL_ASCII, PANEL_R40 };

	explicit MemView(QWidget *parent = nullptr);
	~MemView();
	void connect_emu(EmuModel *emu);

public slots:
	void update_contents(int nb, int addr);
	void update_contents_no_nb(int new_line);

private slots:
	void slot_state_changed(int state);
	void slot_reg_changed(int reg, uint16_t val);

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

	void set_font(QString name, int size = 0);
	void update_font_related_dimensions();
	void update_scroll_range();
	int calculate_scroll_lines(int angleDelta);

protected:
	void paintEvent(QPaintEvent *event) override;
	void resizeEvent(QResizeEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;
	void enterEvent(QEnterEvent *event) override;
	void leaveEvent(QEvent *event) override;
	void keyPressEvent(QKeyEvent *event) override;
};

#endif // MEMVIEW_H

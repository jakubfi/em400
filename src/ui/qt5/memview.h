#ifndef MEMVIEW_H
#define MEMVIEW_H

#include <QWidget>
#include "emumodel.h"

// -----------------------------------------------------------------------
class MemView : public QWidget {

	Q_OBJECT

public:
	explicit MemView(QWidget *parent = nullptr);
	~MemView();
	void connect_emu(EmuModel *emu) { e = emu; }

public slots:
	void update_contents(int nb, int addr);

private:
	int fuzziness = 5;
	int total_lines = 0;
	int bottom, right;
	int offset;
	int half_font_width;
	int line_height;
	int interline;
	int addr_x_start, addr_y_start;
	int addr_len; // characters
	int mem_x_start, mem_y_start;
	int divider_x_pos;

	int words_per_line;

	int cnb, caddr;

	EmuModel *e;
	QFont font;
	int font_height, font_width, font_descent;

	void set_font(QString name, int size=0);
	void update_font_related_dimensions();
	void internal_update_contents();

protected:
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);
	void wheelEvent(QWheelEvent *event);

};

#endif // MEMVIEW_H

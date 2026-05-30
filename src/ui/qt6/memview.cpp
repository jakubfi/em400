#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <QScrollBar>
#include <emdas.h>
#include "memview.h"


// -----------------------------------------------------------------------
MemView::MemView(QWidget *parent) :
	QWidget(parent)
{
	cnb = caddr = 0;
	bottom = 100;
	right = 100;
    words_per_line = 16;

    set_font("Monospace", 12);

	setFocusPolicy(Qt::WheelFocus);

	scroll = new QScrollBar(Qt::Vertical, this);
	scroll->setMinimum(0);
	scroll->setSingleStep(1); // one line
	scroll->setVisible(false);
	update_scroll_range();

	connect(scroll, &QScrollBar::valueChanged, this, &MemView::update_contents_no_nb);
}

// -----------------------------------------------------------------------
MemView::~MemView()
{
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
	font_descent = fm.descent();

	update_font_related_dimensions();
}

// -----------------------------------------------------------------------
void MemView::update_font_related_dimensions()
{
	offset = 5; // pixels
	interline = 4;
	half_font_width = font_width / 2;
	line_height = font_height + interline;

	addr_x_start = half_font_width;
	addr_y_start = font_height;
	addr_len = 5; // characters

	mem_x_start = addr_x_start + font_width * (addr_len + 1);
	mem_y_start = addr_y_start;

	divider_x_pos = mem_x_start - font_width;
}

// -----------------------------------------------------------------------
void MemView::update_scroll_range()
{
	// scrollbar works in lines; cap so the last line of memory lands at the
	// bottom of the view (no scrolling past 0xffff / wrap-around)
	const int mem_lines = 0x10000 / words_per_line;
	const int visible_lines = bottom / line_height; // fully visible lines
	int max_top_line = mem_lines - visible_lines;
	if (max_top_line < 0) max_top_line = 0;
	scroll->setMaximum(max_top_line);
}

// -----------------------------------------------------------------------
void MemView::internal_update_contents()
{

}

// -----------------------------------------------------------------------
void MemView::update_contents(int new_nb, int new_addr)
{
	cnb = new_nb;
	caddr = new_addr;
	scroll->setValue(caddr / words_per_line);
	internal_update_contents();
	update();
}

// -----------------------------------------------------------------------
void MemView::update_contents_no_nb(int new_line)
{
	int new_addr = new_line * words_per_line;
	if (new_addr == caddr) return;
	caddr = new_addr;
	internal_update_contents();
	update();
}

// -----------------------------------------------------------------------
void MemView::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	// backgroud
	painter.fillRect(event->rect(), this->palette().color(QPalette::Base));

	int x, y;
	for (y=0 ; y<total_lines ; y++) {
		// address
		int addr = caddr + y * words_per_line;
		if (addr > 0xffff) break; // don't draw past the end of memory
		QString addr_str = QString("%1").arg((uint16_t)addr, 4, 16, QLatin1Char('0'));
		font.setBold(true);
		painter.setFont(font);
		painter.drawText(addr_x_start, addr_y_start + y * line_height, addr_str);

		// values
		for (x=0 ; x<words_per_line ; x++) {
			int val = e->get_mem(cnb, caddr + y * words_per_line + x);
			QString val_str;
			if (val >= 0) {
				val_str = QString("%1").arg(val, 4, 16);
			} else {
				val_str = QString("----");
			}
			font.setBold(false);
			painter.setFont(font);
			painter.setPen(this->palette().color(QPalette::Text));
			painter.drawText(mem_x_start + x * 5 * font_width, mem_y_start + y * line_height, val_str);
		}
	}

	// divider line
	painter.setPen(this->palette().color(QPalette::Highlight));
	painter.drawLine(divider_x_pos, 0, divider_x_pos, bottom);

	// frame around the widget
	painter.setPen(this->palette().color(QPalette::Mid));
	painter.drawRect(0, 0, right, bottom);
}

// -----------------------------------------------------------------------
void MemView::resizeEvent(QResizeEvent *event)
{
//	int old_dasm_total_lines = dasm_total_lines;

	bottom = this->rect().bottom(); // view height, in pixels
	right = this->rect().right(); // view width, in pixels
	total_lines = bottom / line_height + 1; // +1 for the line at the bottom edge of the window

	update_scroll_range();
	scroll->setPageStep(total_lines);
	scroll->setGeometry(width() - scroll->sizeHint().width(), 0, scroll->sizeHint().width(), height());

	QWidget::resizeEvent(event);
}

// -----------------------------------------------------------------------
int MemView::calculate_scroll_lines(int angleDelta)
{
	const int native_wheel_step = 120; // standard OS notch size
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

	scroll->setValue(scroll->value() - delta_lines); // in lines; triggers update_contents_no_nb()

	event->accept();
	update();
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

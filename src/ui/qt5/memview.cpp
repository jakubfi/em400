#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <emdas.h>
#include "memview.h"


// -----------------------------------------------------------------------
MemView::MemView(QWidget *parent) :
	QWidget(parent)
{
	cnb = caddr = 0;
	bottom = 100;
	right = 100;
	words_per_line = 8;

	set_font("Monospace");
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
void MemView::internal_update_contents()
{

}

// -----------------------------------------------------------------------
void MemView::update_contents(int new_nb, int new_addr)
{
	cnb = new_nb;
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
	painter.setFont(font);

	int x, y;
	for (y=0 ; y<total_lines ; y++) {
		// address
		int addr = caddr + y * words_per_line;
		font.setBold(true);
		painter.setFont(font);
		painter.setPen(this->palette().color(QPalette::PlaceholderText));
		painter.drawText(addr_x_start, y * line_height, QString("%1").arg((uint16_t)addr, 4, 16, QLatin1Char('0')));

		for (x=0 ; x<words_per_line ; x++) {
			font.setBold(false);
			painter.setFont(font);
			painter.setPen(this->palette().color(QPalette::Text));
			painter.drawText(mem_x_start + x * 5 * font_width, y * line_height, QString("%1").arg(e->get_mem(cnb, caddr + y * words_per_line + x), 4, 16));
		}
	}

	// divider line
	painter.setPen(this->palette().color(QPalette::Background));
	painter.drawLine(divider_x_pos, 0, divider_x_pos, bottom);

//	// disassembly
//	QColor bar_color;
//	int y = mem_y_start;
//	Q_FOREACH (const AsmLine &l, listing) {
//		// bar for IC location
//		if (l.addr == e->get_reg(ECTL_REG_IC)) {
//			// change bar color if "P" flag is set (instruction won't be executed)
//			if (e->get_reg(ECTL_REG_P)) bar_color = QColor(Qt::red).lighter();
//			else bar_color = this->palette().color(QPalette::Mid);
//			painter.fillRect(QRect(0, y+font_descent+1, right+1, -(font_height+interline)), bar_color);
//		}


//		// program line

//		y += line_height;
//	}

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

//	if (caddr >= 0) {
//		int rows_delta = dasm_total_lines - old_dasm_total_lines;
//		if (rows_delta > 0) { // more rows visible
//			int i = append(rows_delta, listing);
//			prepend(rows_delta - i, listing);
//		} else { // less rows visible
//			while (rows_delta++ < 0) listing.removeLast();
//		}
//	}
	QWidget::resizeEvent(event);
}

// -----------------------------------------------------------------------
void MemView::wheelEvent(QWheelEvent *event)
{
//	int wheel_step = 1;

//	if (listing.empty()) return;

//	int delta = wheel_step * (event->angleDelta().y() / 120);
//	if (delta > 0) { // backwards
//		if (listing.first().addr > 0) {
//			int items = prepend(delta, listing);
//			while (items-- > 0) listing.removeLast();
//		}
//	} else { // forward
//		if (listing.last().addr < 0xffff) {
//			int items = append(-delta, listing);
//			while (items-- > 0) listing.removeFirst();
//		}
//	}

//	QWidget::wheelEvent(event);
//	update();
}

#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <emdas.h>
#include "dasmview.h"


// -----------------------------------------------------------------------
int dbg_mem_get(int nb, uint16_t addr, uint16_t *data)
{
	return ectl_mem_get(nb, addr, data, 1);
}

// -----------------------------------------------------------------------
DasmView::DasmView(QWidget *parent) :
	QWidget(parent)
{
	emd = emdas_create(EMD_ISET_MX16, dbg_mem_get);
	emdas_set_nl(emd, '\0');
	emdas_set_features(emd, EMD_FEAT_UMNEMO);
	emdas_set_tabs(emd, 0, 0, 5, 4);

	cnb = caddr = -1;
	bottom = 100;
	right = 100;

	set_font("Monospace");
}

// -----------------------------------------------------------------------
DasmView::~DasmView()
{
	emdas_destroy(emd);
}

// -----------------------------------------------------------------------
void DasmView::set_font(QString name, int size)
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
void DasmView::update_font_related_dimensions()
{
	offset = 5; // pixels
	interline = 4;
	half_font_width = font_width / 2;
	line_height = font_height + interline;

	addr_x_start = half_font_width;
	addr_y_start = font_height;
	addr_len = 5; // characters

	dasm_x_start = addr_x_start + font_width * (addr_len + 1);
	dasm_y_start = addr_y_start;

	divider_x_pos = dasm_x_start - font_width;
}

// -----------------------------------------------------------------------
AsmLine DasmView::dasm_exact(int nb, int addr)
{
	int words = emdas_dasm(emd, nb, addr);
	char *buf = emdas_get_buf(emd);
	return AsmLine(nb, addr, words, buf);
}

// -----------------------------------------------------------------------
AsmLine DasmView::dasm_fuzzy(int nb, int addr)
{
	// with fuzzy dasm, addr may not necesarily point to an instruction.
	// it may as well point to the argument of an instruction.
	// do a fuzzy search starting 'fuzziness' words earlier
	int probe_addr = addr - fuzziness;
	if (probe_addr < 0) probe_addr = 0;

	while (true) {
		AsmLine a = dasm_exact(nb, probe_addr);
		if ((addr >= a.addr) && (addr < a.addr + a.length)) return a;
		probe_addr += a.length;
	}
}

// -----------------------------------------------------------------------
int DasmView::prepend(int i, QList<AsmLine>& l)
{
	if (l.length() <= 0) return 0;

	// it's unknow where to start to get 'i' lines of code.
	// start searching i*2 words earlier (worst case for 2-word instructions)
	int search_addr = l.first().addr - i*2;
	if (search_addr < 0) search_addr = 0;

	QList<AsmLine> tmp_l;

	tmp_l.append(dasm_fuzzy(cnb, search_addr++));
	while (tmp_l.last().addr + tmp_l.last().length < l.first().addr) {
		tmp_l.append(dasm_exact(cnb, search_addr++));
	}

	// remove extra items at the begining
	while (tmp_l.length() > i) tmp_l.removeFirst();

	int count = tmp_l.count();
	tmp_l.append(l);
	l.swap(tmp_l);

	return count;
}

// -----------------------------------------------------------------------
int DasmView::append(int i, QList<AsmLine>& l)
{
	int c = 0;

	if ((l.length() <= 0) && (i > 0)) {
		l.append(dasm_fuzzy(cnb, caddr));
		c++;
	}

	for (; c<i ; c++) {
		AsmLine &last = l.last();
		if (last.addr + last.length > 0xffff) break;
		l.append(dasm_exact(last.nb, last.addr + last.length));
	}

	return c;
}

// -----------------------------------------------------------------------
void DasmView::internal_update_contents()
{
	listing.clear();
	int i = append(dasm_total_lines, listing);
	// if the list does not fill all the space, prepend the difference
	prepend(dasm_total_lines - i, listing);
}

// -----------------------------------------------------------------------
void DasmView::update_contents(int new_nb, int new_addr)
{
	cnb = new_nb;
	caddr = new_addr;
	internal_update_contents();
	update();
}

// -----------------------------------------------------------------------
void DasmView::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.fillRect(event->rect(), this->palette().color(QPalette::Base));
	painter.setFont(font);

	// divider line
	painter.setPen(this->palette().color(QPalette::Background));
	painter.drawLine(divider_x_pos, 0, divider_x_pos, bottom);

	// disassembly
	QColor bar_color;
	int y = dasm_y_start;
	Q_FOREACH (const AsmLine &l, listing) {
		// bar for IC location
		if (l.addr == e->get_reg(ECTL_REG_IC)) {
			// change bar color if "P" flag is set (instruction won't be executed)
			if (e->get_reg(ECTL_REG_P)) bar_color = QColor(Qt::red).lighter();
			else bar_color = this->palette().color(QPalette::Mid);
			painter.fillRect(QRect(0, y+font_descent+1, right+1, -(font_height+interline)), bar_color);
		}

		// address
		font.setBold(true);
		painter.setFont(font);
		painter.setPen(this->palette().color(QPalette::PlaceholderText));
		painter.drawText(addr_x_start, y, QString("%1").arg((uint16_t)l.addr, 4, 16, QLatin1Char('0')));

		// program line
		font.setBold(false);
		painter.setFont(font);
		painter.setPen(this->palette().color(QPalette::Text));
		painter.drawText(dasm_x_start, y, l.text);

		y += line_height;
	}
}

// -----------------------------------------------------------------------
void DasmView::resizeEvent(QResizeEvent *event)
{
	int old_dasm_total_lines = dasm_total_lines;

	bottom = this->rect().bottom(); // view height, in pixels
	right = this->rect().right(); // view width, in pixels
	dasm_total_lines = bottom / line_height + 1; // +1 for the line at the bottom edge of the window

	if (caddr >= 0) {
		int rows_delta = dasm_total_lines - old_dasm_total_lines;
		if (rows_delta > 0) { // more rows visible
			int i = append(rows_delta, listing);
			prepend(rows_delta - i, listing);
		} else { // less rows visible
			while (rows_delta++ < 0) listing.removeLast();
		}
	}
	QWidget::resizeEvent(event);
}

// -----------------------------------------------------------------------
void DasmView::wheelEvent(QWheelEvent *event)
{
	int wheel_step = 1;
	int items;

	if (listing.empty()) return;

	if (event->delta() > 0) { // backwards
		if (listing.first().addr > 0) {
			items = prepend(wheel_step, listing);
			while (wheel_step-- >= items) listing.removeLast();
		}
	} else { // forward
		if (listing.last().addr < 0xffff) {
			items = append(wheel_step, listing);
			while (wheel_step-- >= items) listing.removeFirst();
		}
	}

	QWidget::wheelEvent(event);
	update();
}

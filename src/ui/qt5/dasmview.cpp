#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <emdas.h>
#include "dasmview.h"

// -----------------------------------------------------------------------
AsmLine::AsmLine(int nb, int addr, int length, const char *text) :
	nb(nb),
	addr(addr),
	length(length),
	text(text)
{

}

// -----------------------------------------------------------------------
AsmLine::AsmLine(int nb, int addr, int length, QString text) :
	nb(nb),
	addr(addr),
	length(length),
	text(text)
{

}

// -----------------------------------------------------------------------
AsmLine::~AsmLine()
{

}

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

	nb = addr = -1;
	bottom = 100;
	right = 100;
	context = 1;

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
AsmLine DasmView::dasm_exact(int nb, int addr)
{
	int words = emdas_dasm(emd, nb, addr);
	char *buf = emdas_get_buf(emd);
	return AsmLine(nb, addr, words, buf);
}

// -----------------------------------------------------------------------
AsmLine DasmView::dasm_fuzzy(int nb, int addr, int fuzziness)
{
	int probe_addr = addr - fuzziness;
	if (probe_addr < 0) probe_addr = 0;

	while (true) {
		AsmLine a = dasm_exact(nb, probe_addr);
		if ((addr >= a.addr) && (addr < a.addr + a.length)) return a;
		probe_addr += a.length;
	}
}

// -----------------------------------------------------------------------
int DasmView::prepend(int i)
{
	return 0;
}

// -----------------------------------------------------------------------
int DasmView::append(int i)
{
	if (listing.length() <= 0) return 0;

	int c;
	for (c=0 ; c<i ; c++) {
		AsmLine &last = listing.last();
		if (last.addr + last.length > 0xffff) break;
		listing.append(dasm_exact(last.nb, last.addr));
	}

	return c;
}

// -----------------------------------------------------------------------
void DasmView::internal_update_contents()
{
	// Each instruction can be 1 or 2 words long.
	// It is unknown:
	//  * whether it is 1 or 2 until disassembly is done,
	//  * how 'context', given in lines, translates to word count,
	//  * if 'addr' is at start of an instruction.

	// Start disassembling context*2 words earlier, plus 4 for uncertainty of
	// "did we start on the instruction or its argument, so things can align.
	int dasm_addr = addr - context*2 - 4;
	if (dasm_addr < 0) dasm_addr = 0;

	// we need this many lines of actual disassembled program
	int lines_required = dasm_total_lines;
	int lines_disassembled = 0;
	// index (in the list) of the item containing disassembly @ requested address
	int addr_requested_index = -1;

	char *buf = emdas_get_buf(emd);

	listing.clear();
	while (lines_required > 0) {
		// disassemble and append
		uint16_t words = emdas_dasm(emd, nb, dasm_addr);
		listing.append(AsmLine(nb, dasm_addr, words, buf));
		lines_disassembled++;

		// if requested address has been found, each disassembled line counts towards lines_required
		if (addr_requested_index >= 0) {
			lines_required--;
		// if requested address has not yet been found, check if now's the time
		} else {
			// this disassembly was spot on the addr
			if (dasm_addr == addr) {
				addr_requested_index = lines_disassembled-1;
				lines_required -= qMin(context, lines_disassembled);
			// this disassembly was after the addr, and exact addr was not found earlier
			// that means addr was on the argument of the previous instruction, so index for addr lands on previous instruction
			} else if (dasm_addr > addr) {
				addr_requested_index = lines_disassembled-2;
				lines_required -= qMin(context, lines_disassembled-1);
			}
		}

		dasm_addr += words;
	}

	// remove all lines before context
	while (addr_requested_index > context) {
		listing.removeFirst();
		addr_requested_index--;
	}
}

// -----------------------------------------------------------------------
void DasmView::update_contents(int new_nb, int new_addr)
{
	nb = new_nb;
	addr = new_addr;
	internal_update_contents();
	update();
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
	// view height, in pixels
	bottom = this->rect().bottom();
	// view width, in pixels
	right = this->rect().right();
	// we need this many lines of output to fill up the window
	// (+1 for the line at the edge of the window)
	dasm_total_lines = bottom / line_height + 1;
	// linex of context before active line of the disassembly
	context = dasm_total_lines / 3;
	if (addr >= 0) {
		internal_update_contents();
	}
	QWidget::resizeEvent(event);
}

// -----------------------------------------------------------------------
void DasmView::wheelEvent(QWheelEvent *event)
{
	int wheel_step = 3;
	qDebug() << event;
	if (addr >= 0) {
		if(event->delta() > 0)
		{
			qDebug() << ">0" << addr;
			if (addr > 0) {
				addr -= wheel_step;
				if (addr < 0) addr = 0;
			}
		} else {
			qDebug() << "<0" << addr;
			addr += wheel_step;
		}
		internal_update_contents();
	}
	QWidget::wheelEvent(event);
	update();
}

#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <QScrollBar>
#include <emdas.h>
#include "dasmview.h"


// -----------------------------------------------------------------------
static int dbg_mem_get(int nb, uint16_t addr, uint16_t *data)
{
	return ectl_mem_read_n(nb, addr, data, 1);
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

	set_font("Monospace");

	setFocusPolicy(Qt::WheelFocus);

	scroll = new QScrollBar(Qt::Vertical, this);
	scroll->setMinimum(0);
	scroll->setMaximum(65535);
	scroll->setVisible(false);

	connect(scroll, &QScrollBar::valueChanged, this, &DasmView::update_contents_no_nb);
}

// -----------------------------------------------------------------------
DasmView::~DasmView()
{
	emdas_destroy(emd);
	delete scroll;
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
	line_height = font_height + interline;
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

	const int count = tmp_l.count();
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
	const int i = append(dasm_total_lines, listing);
	// if the list does not fill all the space, prepend the difference
	prepend(dasm_total_lines - i, listing);
	scroll->setValue(caddr);
	update();
}

// -----------------------------------------------------------------------
void DasmView::update_contents(int new_nb, int new_addr)
{
//	if ((new_nb == cnb) && (new_addr == caddr)) return;
	cnb = new_nb;
	caddr = new_addr;
	internal_update_contents();
}

// -----------------------------------------------------------------------
void DasmView::update_contents_no_nb(int new_addr)
{
	if (new_addr == caddr) return;
	caddr = new_addr;
	internal_update_contents();
}

// -----------------------------------------------------------------------
void DasmView::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	const int line_y_start = font_height;
	const int addr_x_start = font_width / 2;
	const int divider_x_pos = addr_x_start + font_width * (addr_len + 1);
	const int dasm_x_start = divider_x_pos + font_width;

	// background
	painter.fillRect(event->rect(), palette().color(QPalette::Base));

	// addr-code divider line
	painter.setPen(palette().color(QPalette::Background));
	painter.drawLine(divider_x_pos, 0, divider_x_pos, height());

	// disassembly
	QColor bar_color;
	int y = line_y_start;
	Q_FOREACH (const AsmLine &l, listing) {
		const bool at_ic = l.addr == e->get_reg(ECTL_REG_IC);

		// bar for IC location
		if (at_ic) {
			// change bar color if "P" flag is set (instruction won't be executed)
			if (e->get_reg(ECTL_REG_P)) {
				bar_color = QColor(Qt::red).lighter();
			} else {
				if (hasFocus()) {
					bar_color = palette().color(QPalette::Highlight);
				} else {
					bar_color = palette().color(QPalette::Inactive, QPalette::Highlight);
				}
			}
			painter.fillRect(QRect(0, y+font_descent+1, width()+1, -(font_height+interline)), bar_color);
		}

		// address
		font.setBold(true);
		if (at_ic) painter.setPen(palette().color(QPalette::HighlightedText));
		else painter.setPen(palette().color(QPalette::PlaceholderText));
		painter.drawText(addr_x_start, y, QString("%1").arg((uint16_t)l.addr, 4, 16, QLatin1Char('0')));

		// code
		font.setBold(false);
		if (at_ic) painter.setPen(palette().color(QPalette::HighlightedText));
		else painter.setPen(palette().color(QPalette::Text));
		painter.drawText(dasm_x_start, y, l.text);

		y += line_height;
	}

	// frame around the widget
	painter.setPen(palette().color(QPalette::Mid));
	painter.drawRect(0, 0, width()-1, height()-1);
}

// -----------------------------------------------------------------------
void DasmView::resizeEvent(QResizeEvent *event)
{
	const int old_dasm_total_lines = dasm_total_lines;

	dasm_total_lines = (height() / line_height) + 1; // +1 for the line at the bottom edge of the window

	if (caddr >= 0) {
		int rows_delta = dasm_total_lines - old_dasm_total_lines;
		if (rows_delta > 0) { // more rows visible
			const int i = append(rows_delta, listing);
			prepend(rows_delta - i, listing);
		} else { // less rows visible
			while (rows_delta++ < 0) listing.removeLast();
		}
	}
	scroll->setGeometry(width() - scroll->sizeHint().width(), 0, scroll->sizeHint().width(), height());

	QWidget::resizeEvent(event);
}

// -----------------------------------------------------------------------
void DasmView::wheelEvent(QWheelEvent *event)
{
	if (listing.empty()) return;

	static int tickcount;
	const int wheel_step = 3;
	const int one_line_advance = 120 / wheel_step;

	tickcount += event->angleDelta().y();
	const int delta = tickcount / one_line_advance;
	tickcount -= delta * one_line_advance;

	if (delta > 0) { // backwards
		if (listing.first().addr > 0) {
			int items = prepend(delta, listing);
			while (items-- > 0) listing.removeLast();
		}
	} else { // forward
		if (listing.last().addr < 0xffff) {
			int items = append(-delta, listing);
			while (items-- > 0) listing.removeFirst();
		}
	}
	scroll->setValue(listing.first().addr); // TODO: this should all be in one place...
	QWidget::wheelEvent(event);
	update();

}

// -----------------------------------------------------------------------
QSize DasmView::minimumSizeHint() const
{
	return QSize(dasm_line_length * font_width, -1);
}

// -----------------------------------------------------------------------
QSize DasmView::sizeHint() const
{
	return minimumSizeHint();
}

// -----------------------------------------------------------------------
void DasmView::enterEvent(QEvent *event)
{
	scroll->setVisible(true);
}

// -----------------------------------------------------------------------
void DasmView::leaveEvent(QEvent *event)
{
	scroll->setVisible(false);
}

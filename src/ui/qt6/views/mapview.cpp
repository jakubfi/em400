#include <QPainter>
#include <QFontMetrics>
#include <QFontInfo>
#include <QPalette>
#include "mapview.h"
#include "libem400.h"
#include "theme.h"

// -----------------------------------------------------------------------
MapView::MapView(EmuModel *emu, QWidget *parent) :
	QWidget(parent),
	e(emu)
{
	// the app default UI font, one point smaller (matches the other dock views)
	fnt = font();
	fnt.setPointSize(QFontInfo(fnt).pointSize() - 1);

	compute_geometry();

	// hover-cursor feedback needs move events without a button held
	setMouseTracking(true);

	for (int seg=0 ; seg<SEGS ; seg++) map[seg] = e->get_mem_map(seg);

	connect(e, &EmuModel::signal_mem_map_changed, this, &MapView::slot_map_changed);
}

// -----------------------------------------------------------------------
// All sizing derives from the (fixed) font. A cell is a square sized to hold a
// single hex digit comfortably; the gutter holds the row label.
void MapView::compute_geometry()
{
	QFontMetrics fm(fnt);
	margin = 6;
	gap = 2;
	// ~30% smaller than the old digit-height square; floored so it never collapses
	cell = qMax(8, qMax(fm.height(), fm.horizontalAdvance("0") + 6) * 7 / 10);
	// labels are single hex digits (0..f); size the gutter to one digit (plus a
	// little slack so glyph bearings are not clipped) and a 6px gap to the grid
	lbl_w = fm.horizontalAdvance("0") + 4 + 6;
	hdr_h = fm.height() + 2;
}

// -----------------------------------------------------------------------
QSize MapView::content_size() const
{
	int w = margin + lbl_w + grid_w() + margin;
	int h = grid_y() + SEGS * cell + (SEGS - 1) * gap + margin;
	return QSize(w, h);
}

// -----------------------------------------------------------------------
void MapView::slot_map_changed(int seg, uint16_t m)
{
	if (seg < 0 || seg >= SEGS) return;
	map[seg] = m;
	update();
}

// -----------------------------------------------------------------------
// Map a widget-space point to a page cell. Returns false for the headers,
// gutter and inter-cell gaps so only the cell squares are hot.
bool MapView::hit_cell(const QPoint &pos, int &seg, int &page) const
{
	int cx = pos.x() - grid_x();
	int cy = pos.y() - grid_y();
	if (cx < 0 || cy < 0) return false;

	int col = cx / (cell + gap);
	int row = cy / (cell + gap);
	if (col >= PAGES || row >= SEGS) return false;

	if (cx - col * (cell + gap) >= cell) return false;
	if (cy - row * (cell + gap) >= cell) return false;

	seg = row;
	page = col;
	return true;
}

// -----------------------------------------------------------------------
// A click on a page cell jumps the memory view to that page's start address
// in the clicked segment.
void MapView::mousePressEvent(QMouseEvent *ev)
{
	int seg, page;
	if (ev->button() == Qt::LeftButton && hit_cell(ev->pos(), seg, page)) {
		emit signal_page_clicked(seg, page * PAGE_WORDS);
		return;
	}
	QWidget::mousePressEvent(ev);
}

// -----------------------------------------------------------------------
// Over a clickable page cell: show a pointing-hand cursor and a tooltip
// hinting where a click will take the memory view (seg:page-start-address).
void MapView::mouseMoveEvent(QMouseEvent *ev)
{
	int seg, page;
	if (hit_cell(ev->pos(), seg, page)) {
		setCursor(Qt::PointingHandCursor);
		setToolTip(tr("click to set memory view to %1:%2")
			.arg(seg, 0, 16)
			.arg(page * PAGE_WORDS, 4, 16, QLatin1Char('0')));
	} else {
		setCursor(Qt::ArrowCursor);
		setToolTip(QString());
	}
	QWidget::mouseMoveEvent(ev);
}

// -----------------------------------------------------------------------
void MapView::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.setFont(fnt);

	// allocated pages use the amber accent (shared with the interrupt mask boxes)
	QColor on   = em400_mask_color(palette());
	QColor off  = palette().color(QPalette::Mid);
	QColor text = palette().color(QPalette::WindowText);
	QColor dim  = palette().color(QPalette::Disabled, QPalette::WindowText);

	const int gx = grid_x();
	const int gy = grid_y();

	// column headers (page index, 0..f) centred over each cell column
	p.setPen(dim);
	for (int c=0 ; c<PAGES ; c++) {
		int x = gx + c * (cell + gap);
		QRect r(x, margin, cell, hdr_h);
		p.drawText(r, Qt::AlignCenter, QString::number(c, 16));
	}

	for (int row=0 ; row<SEGS ; row++) {
		int y = gy + row * (cell + gap);

		// row label: segment number in hex, right-aligned in the gutter
		p.setPen(text);
		QRect lr(margin, y, lbl_w - 6, cell);
		p.drawText(lr, Qt::AlignRight | Qt::AlignVCenter, QString::number(row, 16));

		for (int c=0 ; c<PAGES ; c++) {
			int x = gx + c * (cell + gap);
			QRect cr(x, y, cell, cell);
			bool alloc = map[row] & (1u << c);
			if (alloc) {
				p.fillRect(cr.adjusted(1, 1, -1, -1), on);
			} else {
				p.setPen(off);
				p.drawRect(cr.adjusted(1, 1, -2, -2));
			}
		}
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent

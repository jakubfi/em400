#include <QPainter>
#include <QFontMetrics>
#include <QFontInfo>
#include <QPalette>
#include "mapview.h"
#include "libem400.h"

// -----------------------------------------------------------------------
MapView::MapView(EmuModel *emu, QWidget *parent) :
	QWidget(parent),
	e(emu)
{
	// the app default UI font, one point smaller (matches the other dock views)
	fnt = font();
	fnt.setPointSize(QFontInfo(fnt).pointSize() - 1);

	compute_geometry();

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
void MapView::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.setFont(fnt);

	QColor on   = palette().color(QPalette::Highlight);
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

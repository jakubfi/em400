#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QWidget>
#include <QFont>
#include <QMouseEvent>
#include "emumodel.h"

// -----------------------------------------------------------------------
// Memory allocation map. 16 segments (rows) x 16 pages (columns); each page
// is 4 K words. A page is "allocated" when its bit is set in em400_mem_map(seg).
// A filled cell = allocated page, an empty outline = free page.
//
// Read-only: the map reflects the memory configuration, which the UI cannot
// change. State is pulled from EmuModel's polling (signal_mem_map_changed).
// -----------------------------------------------------------------------
class MapView : public QWidget {
	Q_OBJECT

public:
	explicit MapView(EmuModel *emu, QWidget *parent = nullptr);

	QSize sizeHint() const override { return content_size(); }
	QSize minimumSizeHint() const override { return content_size(); }

signals:
	// a page cell was clicked: seg is the segment (row), addr is the page's
	// start address within that segment (page * 4 K words)
	void signal_page_clicked(int seg, int addr);

protected:
	void paintEvent(QPaintEvent *ev) override;
	void mousePressEvent(QMouseEvent *ev) override;
	void mouseMoveEvent(QMouseEvent *ev) override;

private slots:
	void slot_map_changed(int seg, uint16_t map);

private:
	static const int SEGS = 16;				// segments (rows)
	static const int PAGES = 16;			// pages per segment (columns)
	static const int PAGE_WORDS = 0x1000;	// 4 K words per page

	EmuModel *e;
	QFont fnt;
	uint16_t map[SEGS] = {0};

	// geometry, computed once from the font (the widget is fixed-size content)
	int cell, gap, lbl_w, hdr_h, margin;

	void compute_geometry();
	QSize content_size() const;
	// map a widget-space point to a page cell; false outside the cell squares
	bool hit_cell(const QPoint &pos, int &seg, int &page) const;
	int grid_w() const { return PAGES * cell + (PAGES - 1) * gap; }
	int grid_x() const { return margin + lbl_w; }
	int grid_y() const { return margin + hdr_h; }
};

#endif // MAPVIEW_H

// vim: tabstop=4 shiftwidth=4 autoindent

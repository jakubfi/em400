//  Copyright (c) 2026 Jakub Filipowicz <jakubf@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef DASMLISTING_H
#define DASMLISTING_H

#include <QWidget>
#include <QScrollBar>
#include "emumodel.h"
#include "asmline.h"

// -----------------------------------------------------------------------
// The scrolling disassembly listing: a plain 0,0-relative painted box. Driven
// externally (segment / follow IC) so the surrounding dock can lay it out and
// control it like any other widget.
class DasmListing : public QWidget {

	Q_OBJECT

public:
	explicit DasmListing(QWidget *parent = nullptr);
	~DasmListing();
	void connect_emu(EmuModel *emu) { e = emu; }

public slots:
	void update_contents(int nb, int addr);
	void set_nb(int nb);
	void set_follow(bool on);

signals:
	// right-click "Locate in Memory View": jump the memory view to this segment
	// and address and highlight the cell there
	void signal_locate_in_memory(int nb, int addr);
	// the view changed what it shows on its own (followed the IC into a new
	// segment / dropped follow); the header controls mirror these
	void nb_changed(int nb);
	void follow_changed(bool on);

private slots:
	void update_contents_no_nb(int new_addr);

private:
	EmuModel *e;

	int cnb, caddr, ic_addr, ic_nb;
	bool follow_ic = true;

	QFont font;
	int font_height, font_width, font_descent; // calculated

	const int fuzziness = 5;			// fuzziness when searching for instruction start address
	const int dasm_line_length = 26;	// length of disassembly line
	int dasm_total_lines = 0;			// lines of disassembly visible
	int line_height;					// disassembly line height in pixels, with space between lines
	const int interline = 4;			// additional interline, in pixels
	const int addr_len = 4;				// address length, in characters

	int wheel_tick_accumulator = 0;
	QScrollBar *scroll;

	struct emdas *emd;
	QList<AsmLine> listing;

	void set_font(QString name, int size=0);
	void internal_update_contents();
	void recenter_on_ic();
	void snap_to_ic();
	int max_first_addr();

	AsmLine dasm_exact(int nb, int addr);
	AsmLine dasm_fuzzy(int nb, int addr);
	int prepend(int i, QList<AsmLine>& l);
	int append(int i, QList<AsmLine>& l);
	int delete_last(int i);
	int delete_first(int i);

	int calculate_scroll_lines(int angleDelta);

	// map a widget-space point to the address of the disassembly line under it
	bool hit_test_line(const QPoint &pos, int &addr) const;

protected:
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);
	void wheelEvent(QWheelEvent *event);
	void contextMenuEvent(QContextMenuEvent *event);
	QSize minimumSizeHint() const;
	QSize sizeHint() const;
	void enterEvent(QEnterEvent *event);
	void leaveEvent(QEvent *event);

};

#endif // DASMLISTING_H

// vim: tabstop=4 shiftwidth=4 autoindent

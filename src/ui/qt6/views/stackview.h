#ifndef STACKVIEW_H
#define STACKVIEW_H

#include <QWidget>
#include <QFont>
#include "emumodel.h"

class QContextMenuEvent;

// -----------------------------------------------------------------------
// Top-of-stack view. The MERA-400 has no call stack; the only software stack
// is the interrupt/EXL frame stack, whose pointer lives at 0x61 in segment 0.
// Each frame is exactly four words pushed on entry to a handler:
//
//     S/SP+0/ = IC      (interrupted program counter)
//     S/SP+1/ = R0      (flags)
//     S/SP+2/ = SR      (status register)
//     S/SP+3/ = SPEC    (interrupt specification from the I/O channel:
//                        device number + the code it sent; 0 for non-I/O sources)
//
// ...and SP is then advanced by 4. So the four words immediately below SP are
// the most recent frame. We show only that: the user almost never cares how
// deep the stack runs, and we cannot know its depth anyway. The view is read
// only and pulled from EmuModel (memory + reg polling).
//
// Two cases are deliberately blank (no meaningful stack to show):
//   - SP == 0:  the program never set up a stack pointer.
//   - SP (or the frame below it) points at unallocated memory.
// -----------------------------------------------------------------------
class StackView : public QWidget {
	Q_OBJECT

public:
	explicit StackView(EmuModel *emu, QWidget *parent = nullptr);
	void refresh_font();

	QSize sizeHint() const override { return content_size(); }
	QSize minimumSizeHint() const override { return content_size(); }

protected:
	void paintEvent(QPaintEvent *ev) override;
	void contextMenuEvent(QContextMenuEvent *ev) override;

private slots:
	void refresh();

private:
	static const int FRAME = 4;     // words shown (one interrupt frame)
	static const int SP_ADDR = 0x61; // stack pointer location, segment 0

	enum State { OK, NO_SP, UNALLOC };

	EmuModel *e;
	QFont fnt;
	QFont hdr_fnt;              // SP header: one point smaller than the rows

	State state = NO_SP;
	uint16_t sp = 0;            // current stack pointer
	uint16_t base = 0;          // address of the topmost shown word (sp - FRAME)
	uint16_t word[FRAME] = {0}; // the four frame words

	// geometry, computed once from the font (fixed-size content)
	int margin, line_h, hdr_h, hdr_top, hdr_line, addr_w, role_w, val_w, gap;

	void apply_font();
	void compute_geometry();
	QSize content_size() const;
	// the hex text under a point (SP / an address / a value), or null if the
	// point is not over a copyable cell. Drives the right-click "Copy".
	QString text_at(QPoint pos) const;
};

#endif // STACKVIEW_H

// vim: tabstop=4 shiftwidth=4 autoindent

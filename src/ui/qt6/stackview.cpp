#include <QPainter>
#include <QFontMetrics>
#include <QFontInfo>
#include <QPalette>
#include <QContextMenuEvent>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include "stackview.h"
#include "libem400.h"
#include "theme.h"

// the four frame slots in ascending address order (sp-4 .. sp-1). The last is
// the interrupt specification read from the I/O channel (device number + the
// code the device sent), or 0 for non-I/O interrupt sources.
static const char *const ROLE[] = { "IC", "R0", "SR", "SPEC" };

// -----------------------------------------------------------------------
StackView::StackView(EmuModel *emu, QWidget *parent) :
	QWidget(parent),
	e(emu)
{
	// monospace, like the disassembly and memory views (addresses/values align).
	// Start from the widget font so we inherit a concrete point size (a bare
	// QFont("Monospace") has no size set, which makes a later setPointSize jump to
	// an unrelated default), then just swap the family.
	fnt = font();
	fnt.setFamily("Monospace");

	compute_geometry();

	// the stack lives in memory, not a register, but it only changes as the
	// machine executes - which also moves the registers. Refreshing on the
	// reg-change signal (slow-timer polled) gives the same cadence as the rest
	// of the debugger without a dedicated poll. State changes (run/stop) also
	// refresh so the frame settles the moment the machine halts.
	connect(e, &EmuModel::signal_reg_changed, this, [this](int, uint16_t){ refresh(); });
	connect(e, &EmuModel::signal_state_changed, this, [this](int){ refresh(); });

	refresh();
}

// -----------------------------------------------------------------------
// Monospace columns sized to their widest content: a 4-hex-digit address, the
// role tag, and a 4-hex-digit value.
void StackView::compute_geometry()
{
	QFontMetrics fm(fnt);
	margin = 6;
	gap = fm.horizontalAdvance("0") + 4;
	line_h = fm.height() + 2;
	addr_w = fm.horizontalAdvance("0000");
	role_w = fm.horizontalAdvance("SPEC");
	val_w = fm.horizontalAdvance("0000");

	// "SP = xxxx" header: two points smaller than the rows (visually distinct),
	// tightly spaced
	hdr_fnt = fnt;
	hdr_fnt.setPointSize(QFontInfo(fnt).pointSize() - 2);
	hdr_line = QFontMetrics(hdr_fnt).height();
	hdr_top = 6;                 // breathing room above the header
	hdr_h = hdr_top + hdr_line + 5;  // ...and a little gap below before the rows
}

// -----------------------------------------------------------------------
QSize StackView::content_size() const
{
	int w = margin + addr_w + gap + role_w + gap + val_w + margin;
	int h = hdr_h + FRAME * line_h + margin;
	return QSize(w, h);
}

// -----------------------------------------------------------------------
// Re-read SP and the topmost frame from block 0. get_mem() returns -1 when the
// address is unallocated, which is how we detect both "no stack" cases.
void StackView::refresh()
{
	int s = e->get_mem(0, SP_ADDR);
	if (s <= 0) {            // -1 = SP slot unreadable, 0 = never set
		if (state != NO_SP) { state = NO_SP; update(); }
		return;
	}

	sp = (uint16_t)s;
	base = (uint16_t)(sp - FRAME);

	uint16_t w[FRAME];
	for (int i=0 ; i<FRAME ; i++) {
		int v = e->get_mem(0, (uint16_t)(base + i));
		if (v < 0) {         // frame runs into unallocated memory
			if (state != UNALLOC) { state = UNALLOC; update(); }
			return;
		}
		w[i] = (uint16_t)v;
	}

	for (int i=0 ; i<FRAME ; i++) word[i] = w[i];
	state = OK;
	update();
}

// -----------------------------------------------------------------------
void StackView::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.setFont(fnt);

	QColor text = palette().color(QPalette::Text);
	QColor dim  = em400_dim_text_color(palette());

	// the "no meaningful stack" cases: a single centred dim line
	if (state != OK) {
		p.setPen(dim);
		const char *msg = (state == NO_SP) ? "no stack (SP=0)" : "stack unallocated";
		p.drawText(rect(), Qt::AlignCenter, msg);
		return;
	}

	const int x_addr = margin;
	const int x_role = x_addr + addr_w + gap;
	const int x_val  = x_role + role_w + gap;

	QFont bold(fnt); bold.setBold(true);

	// header: where SP points (the next free slot, above the top of the stack)
	p.setFont(hdr_fnt);
	p.setPen(dim);
	p.drawText(QRect(margin, hdr_top, x_val + val_w - margin, hdr_line),
		Qt::AlignLeft | Qt::AlignVCenter,
		QString("SP = %1").arg(sp, 4, 16, QChar('0')));

	// drawn like a stack: top of stack (highest address, sp-1) at the top, just
	// under SP; the frame runs down to IC (sp-4) at the bottom. word[]/ROLE[] are
	// stored in ascending address order, so display row i is slot FRAME-1-i.
	for (int i=0 ; i<FRAME ; i++) {
		int slot = FRAME - 1 - i;
		int y = hdr_h + i * line_h;
		QRect ra(x_addr, y, addr_w, line_h);
		QRect rr(x_role, y, role_w, line_h);
		QRect rv(x_val,  y, val_w,  line_h);

		uint16_t addr = (uint16_t)(base + slot);

		// addresses bold (matches dasm/mem), location labels dim, values plain
		p.setFont(bold);
		p.setPen(text);
		p.drawText(ra, Qt::AlignLeft | Qt::AlignVCenter,
			QString("%1").arg(addr, 4, 16, QChar('0')));

		p.setFont(fnt);
		p.setPen(dim);
		p.drawText(rr, Qt::AlignLeft | Qt::AlignVCenter, ROLE[slot]);

		p.setPen(text);
		p.drawText(rv, Qt::AlignRight | Qt::AlignVCenter,
			QString("%1").arg(word[slot], 4, 16, QChar('0')));
	}
}

// -----------------------------------------------------------------------
// Map a point to the hex text it sits on: the SP value (header), an address or
// a value cell. Role labels are not copyable. Returns null when nothing is hit.
QString StackView::text_at(QPoint pos) const
{
	if (state != OK) return QString();

	const int x = pos.x(), y = pos.y();
	const int x_addr = margin;
	const int x_role = x_addr + addr_w + gap;
	const int x_val  = x_role + role_w + gap;

	// header band: the SP value
	if (y >= hdr_top && y < hdr_top + hdr_line && x >= margin && x < x_val + val_w) {
		return QString("%1").arg(sp, 4, 16, QChar('0'));
	}

	if (y < hdr_h) return QString();
	int row = (y - hdr_h) / line_h;
	if (row < 0 || row >= FRAME) return QString();
	int slot = FRAME - 1 - row;

	if (x >= x_addr && x < x_addr + addr_w) {
		return QString("%1").arg((uint16_t)(base + slot), 4, 16, QChar('0'));
	}
	if (x >= x_val && x < x_val + val_w) {
		return QString("%1").arg(word[slot], 4, 16, QChar('0'));
	}
	return QString();
}

// -----------------------------------------------------------------------
// Right-click an address / value / the SP header to copy its hex to clipboard.
void StackView::contextMenuEvent(QContextMenuEvent *ev)
{
	QString txt = text_at(ev->pos());
	if (txt.isNull()) return;

	QMenu menu(this);
	QAction *copy = menu.addAction(tr("Copy  %1").arg(txt));
	connect(copy, &QAction::triggered, this, [txt]{
		QApplication::clipboard()->setText(txt);
	});
	menu.exec(ev->globalPos());
}

// vim: tabstop=4 shiftwidth=4 autoindent

#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFont>
#include <QFontMetrics>
#include <QFontInfo>
#include <QPalette>
#include <QEvent>
#include <QToolTip>
#include "intview.h"
#include "libem400.h"
#include "theme.h"

// -----------------------------------------------------------------------
MaskBox::MaskBox(int bit, QWidget *parent) :
	QFrame(parent),
	sr_bit(bit)
{
}

// -----------------------------------------------------------------------
// Right-click toggles the mask. Left-click is reserved for the interrupt cells,
// so we ignore it here (clicks on the box padding then simply do nothing).
void MaskBox::mousePressEvent(QMouseEvent *ev)
{
	if (ev->button() == Qt::RightButton) emit clicked();
	else QFrame::mousePressEvent(ev);
}

// -----------------------------------------------------------------------
// UI-owned presentation tables (labels + tooltips). The library exposes only
// structure (em400_int_mask_bit); names and translations belong here.
namespace {

struct RowDesc { const char *label; int first, last; };

const RowDesc row_desc[] = {
	{ "HW",  0,  4 },
	{ "SW",  5, 11 },
	{ "I/O", 12, 27 },
	{ "OTH", 28, 31 },
};

const char *abbr[32] = {
	"POW", "PAR", "SEG", "CPUH", "POWIF",
	"TM", "ILL", "DO", "FPUF", "FPOF", "FPE", "SPEC",
	"0", "1", "2", "3", "4", "5", "6", "7",
	"8", "9", "A", "B", "C", "D", "E", "F",
	"OPRQ", "CPUL", "SH", "SL",
};

const char *full[32] = {
	"CPU power loss", "memory parity error", "no memory", "other CPU (high)",
	"interface power loss", "timer or special", "illegal instruction",
	"division overflow", "FP underflow", "FP overflow", "FP error / div by zero",
	"special or timer",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"operator request", "other CPU (low)", "software (high)", "software (low)",
};

} // namespace

// -----------------------------------------------------------------------
IntView::IntView(EmuModel *emu, QWidget *parent) :
	QWidget(parent),
	e(emu)
{
	for (int n=0 ; n<32 ; n++) mask_bit[n] = e->int_mask_bit(n);

	// inherit the app's default (variable-width) UI font, one point smaller;
	// cells are sized per-label so we do not need monospace alignment
	fnt = font();
	fnt.setPointSize(QFontInfo(fnt).pointSize() - 1);
	cell_fnt = fnt;
	cell_fnt.setPointSize(fnt.pointSize() - 1);

	QVBoxLayout *outer = new QVBoxLayout(this);
	outer->setContentsMargins(4, 4, 4, 4);
	outer->setSpacing(3);

	// small help badge: hover shows how to use the widget + the colour legend
	help = new QLabel("?", this);
	help->setFont(fnt);
	help->setAlignment(Qt::AlignCenter);
	help->setCursor(Qt::PointingHandCursor);
	render_help();
	// keep the default hover-with-delay tooltip, but also pop it immediately on a
	// click - users treat a "?" as clickable and a click that does nothing reads as
	// broken (the tooltip otherwise waits ~700ms for a hover that the click cut short)
	help->installEventFilter(this);
	help->setToolTip(tr(
		"Left-click: set/clear an interrupt (RZ)\n"
		"Right-click: toggle the mask for its group (SR)\n"
		"\n"
		"Yellow fill: active, will fire\n"
		"Bold yellow: active but masked off\n"
		"Box border: accent = unmasked, grey = masked\n"
		"No box: non-maskable interrupt"));

	QGridLayout *grid = new QGridLayout();
	grid->setHorizontalSpacing(3);
	grid->setVerticalSpacing(3);
	outer->addLayout(grid);

	int nrows = sizeof(row_desc) / sizeof(row_desc[0]);
	for (int r=0 ; r<nrows ; r++) {
		QLabel *rl = new QLabel(row_desc[r].label, this);
		rl->setFont(fnt);
		grid->addWidget(rl, r, 0, Qt::AlignRight | Qt::AlignVCenter);

		QWidget *roww = new QWidget(this);
		QHBoxLayout *hb = new QHBoxLayout(roww);
		hb->setContentsMargins(0, 0, 0, 0);
		hb->setSpacing(2);

		int n = row_desc[r].first;
		while (n <= row_desc[r].last) {
			int bit = mask_bit[n];
			// gather the run of consecutive interrupts sharing this mask bit
			int m = n;
			while (m <= row_desc[r].last && mask_bit[m] == bit) m++;

			if (bit < 0) {
				// non-maskable (interrupt 0): bare cell, no toggle box
				for (int i=n ; i<m ; i++) {
					cell[i] = make_cell(i);
					hb->addWidget(cell[i]);
				}
			} else {
				MaskBox *box = new MaskBox(bit, roww);
				QHBoxLayout *bl = new QHBoxLayout(box);
				bl->setContentsMargins(1, 1, 1, 1);
				bl->setSpacing(1);
				for (int i=n ; i<m ; i++) {
					cell[i] = make_cell(i);
					bl->addWidget(cell[i]);
				}
				connect(box, &MaskBox::clicked, this, [=]() { toggle_mask(box->sr_bit); });
				boxes.append(box);
				hb->addWidget(box);
			}
			n = m;
		}
		hb->addStretch(1);
		// the HW row is the shortest (POW + 4 single-bit boxes), leaving slack on
		// the right - park the help badge there instead of on a wasteful own row.
		// The RZ hex value was dropped, so the badge is all that lives up here.
		if (r == 0) hb->addWidget(help);
		grid->addWidget(roww, r, 1);
	}
	grid->setColumnStretch(1, 1);

	outer->addStretch(1);

	rz = e->get_rz();
	sr = e->get_reg(EM400_REG_SR);
	render_all();

	connect(e, &EmuModel::signal_rz_changed, this, &IntView::slot_rz_changed);
	connect(e, &EmuModel::signal_reg_changed, this, &IntView::slot_reg_changed);
}

// -----------------------------------------------------------------------
QPushButton * IntView::make_cell(int n)
{
	QFontMetrics fm(cell_fnt);
	QPushButton *b = new QPushButton(abbr[n], this);
	b->setFont(cell_fnt);
	b->setFlat(true);
	b->setCursor(Qt::PointingHandCursor);
	// size each cell to its own label (not a uniform widest-label width), so the
	// 16-channel I/O row does not blow the dock up to 4-5x its needed width
	b->setFixedSize(fm.horizontalAdvance(abbr[n]) + 4, fm.height() + 2);

	QString tip = (n >= 12 && n <= 27) ? tr("I/O channel %1").arg(n - 12) : tr(full[n]);
	if (mask_bit[n] < 0) tip += tr(" (non-maskable)");
	b->setToolTip(tip);

	// left-click toggles the interrupt; right-click (context menu) toggles the
	// mask of the group this interrupt belongs to (so right-click works whether
	// it lands on a cell or on the surrounding box padding)
	connect(b, &QPushButton::clicked, this, [=]() {
		e->set_int(n, !(rz & (1u << (31 - n))));
	});
	b->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(b, &QPushButton::customContextMenuRequested, this, [=]() {
		toggle_mask(mask_bit[n]);
	});
	return b;
}

// -----------------------------------------------------------------------
void IntView::toggle_mask(int sr_bit)
{
	if (sr_bit < 0) return; // non-maskable interrupt has no mask bit
	e->set_reg(EM400_REG_SR, sr ^ (1u << sr_bit));
}

// -----------------------------------------------------------------------
// Three states from RZ + SR: pending (RZ bit), unmasked (gating SR bit, or
// non-maskable), and the will-fire case (both) which gets the bright fill.
// "Pending" red is a fixed semantic colour (reads on both light and dark);
// the inactive colour comes from the palette so it tracks the theme.
void IntView::render_cell(int n)
{
	bool pending = rz & (1u << (31 - n));
	int bit = mask_bit[n];
	bool unmasked = (bit < 0) || ((sr >> bit) & 1);

	// "this is on": the yellow accent marks pending interrupts (matching the
	// allocation map); dark text rides on the solid fill, the accent itself is
	// the bold text when masked-but-pending. The mask border (render_box) is the
	// same yellow when the gate is open, so a fully-yellow cell = will fire.
	QString yellow = em400_mask_color(palette()).name();
	QString yellow_text = palette().color(QPalette::HighlightedText).name();
	QString dim = em400_dim_text_color(palette()).name();

	QString ss;
	if (pending && unmasked) {
		// set and will fire - solid yellow fill
		ss = QString("QPushButton{background:%1; color:%2; border:none; border-radius:2px;}").arg(yellow, yellow_text);
	} else if (pending) {
		// set in RZ but masked off - bold yellow text, no fill (pending yet suppressed)
		ss = QString("QPushButton{background:transparent; color:%1; border:none; font-weight:bold;}").arg(yellow);
	} else {
		ss = QString("QPushButton{background:transparent; color:%1; border:none;}").arg(dim);
	}
	cell[n]->setStyleSheet(ss);
}

// -----------------------------------------------------------------------
// Solid 1px border either way (constant width avoids a reflow when the mask
// toggles; dashed looked broken at the corners). The contrast is in the
// colour: the amber accent when unmasked, a quiet palette Mid grey when masked
// so the box recedes.
void IntView::render_box(MaskBox *b)
{
	bool unmasked = (sr >> b->sr_bit) & 1;
	QColor c = unmasked ? em400_mask_color(palette())
	                    : palette().color(QPalette::Mid);
	b->setStyleSheet(QString("MaskBox{border:1px solid %1; border-radius:3px;}").arg(c.name()));
}

// -----------------------------------------------------------------------
// The "?" badge: a rounded border with the glyph in the same dimmed-but-
// readable secondary colour (em400_dim_text_color, brighter than the palette's
// Disabled/Mid roles which read too faint here).
// The text colour must be set explicitly - once a QLabel carries a stylesheet
// border it no longer inherits the palette foreground, so without this the
// glyph renders with no usable colour (invisible in both themes). Both colours
// come from the live palette so the badge tracks a runtime theme switch.
void IntView::render_help()
{
	QString c = em400_dim_text_color(palette()).name();
	help->setStyleSheet(QString(
		"QLabel{color:%1; border:1px solid %1; border-radius:3px; padding:0 4px;}").arg(c));
}

// -----------------------------------------------------------------------
void IntView::render_all()
{
	for (int n=0 ; n<32 ; n++) render_cell(n);
	for (MaskBox *b : boxes) render_box(b);
	render_help();
}

// -----------------------------------------------------------------------
// Show the help badge's tooltip immediately on click (in addition to the
// default hover-with-delay behaviour), so an impatient click is not a no-op.
bool IntView::eventFilter(QObject *obj, QEvent *ev)
{
	if (ev->type() == QEvent::MouseButtonPress) {
		QWidget *w = qobject_cast<QWidget*>(obj);
		if (w && !w->toolTip().isEmpty()) {
			QMouseEvent *me = static_cast<QMouseEvent*>(ev);
			QToolTip::showText(me->globalPosition().toPoint(), w->toolTip(), w);
			return true;
		}
	}
	return QWidget::eventFilter(obj, ev);
}

// -----------------------------------------------------------------------
// Re-render when the palette/style changes so the theme-derived colours
// (mask borders, dim text) follow a light/dark switch at runtime.
void IntView::changeEvent(QEvent *ev)
{
	QWidget::changeEvent(ev);
	if (ev->type() == QEvent::PaletteChange || ev->type() == QEvent::StyleChange) {
		render_all();
	}
}

// -----------------------------------------------------------------------
void IntView::slot_rz_changed(uint32_t val)
{
	rz = val;
	for (int n=0 ; n<32 ; n++) render_cell(n);
}

// -----------------------------------------------------------------------
void IntView::slot_reg_changed(int reg, uint16_t val)
{
	if (reg != EM400_REG_SR) return;
	sr = val;
	for (int n=0 ; n<32 ; n++) render_cell(n);
	for (MaskBox *b : boxes) render_box(b);
}

#include <QFont>
#include <QLabel>
#include <QGridLayout>
#include <QEvent>
#include "regview.h"
#include "theme.h"
#include "libem400.h"

// -----------------------------------------------------------------------
RegCompact::RegCompact(EmuModel *emu, Kind kind, QWidget *parent) :
	QWidget(parent),
	e(emu)
{
	if (kind == USER) {
		for (int i=0 ; i<8 ; i++) {
			rows.append({EM400_REG_R0 + i, QString("R%1").arg(i), true});
		}
	} else {
		// order matches the control-panel rotary (the EM400_REG enum order), so the
		// table shows exactly what the rotary register-select exposes. decimal is
		// shown only where the value is numeric data (AC, KB); for addresses/opaque
		// regs (IC, AR, IR, SR, RZ) a signed/decimal reading is meaningless so the
		// row is hex/oct only. RZ here is the 16-bit value the rotary shows
		// (int_get_nchan, the non-channel bits); the full 32-bit interrupt decode
		// lives in the dedicated IntView widget.
		rows.append({EM400_REG_IC, "IC", false});
		rows.append({EM400_REG_AC, "AC", true});
		rows.append({EM400_REG_AR, "AR", false});
		rows.append({EM400_REG_IR, "IR", false});
		rows.append({EM400_REG_SR, "SR", false});
		rows.append({EM400_REG_RZ, "RZ", false});
		rows.append({EM400_REG_KB, "KB", true});
	}

	bool any_dec = false;
	for (const Row &r : rows) any_dec |= r.has_dec;

	int n = rows.size();
	num.resize(n);
	dec.resize(n);
	ghost.resize(n);
	cur.resize(n);

	QFont mono("Monospace");

	QGridLayout *grid = new QGridLayout(this);
	grid->setContentsMargins(4, 4, 4, 4);
	grid->setHorizontalSpacing(6);
	grid->setVerticalSpacing(2);

	// column 1 header is the hex/oct toggle button; all rows switch together
	btn_radix = new QPushButton("hex", this);
	btn_radix->setFont(mono);
	btn_radix->setFlat(true);
	btn_radix->setMaximumHeight(QFontMetrics(mono).height() + 6);
	connect(btn_radix, &QPushButton::clicked, this, &RegCompact::toggle_radix);
	grid->addWidget(btn_radix, 0, 1);

	if (any_dec) {
		QLabel *dec_hdr = new QLabel("dec", this);
		dec_hdr->setFont(mono);
		dec_hdr->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		grid->addWidget(dec_hdr, 0, 2);
	}

	// size the fields to their widest possible content (octal "177777" / signed
	// "-32768") with light padding, and pin the height near the font height with
	// trimmed text margins - the right-hand dock stack is vertical-space starved.
	QFontMetrics fm(mono);
	int num_w = fm.horizontalAdvance("177777") + 10;
	int dec_w = fm.horizontalAdvance("-32768") + 10;
	int ghost_w = fm.horizontalAdvance("65535") + 6;
	int field_h = fm.height() + 4;

	// pin the toggle button to the field width so column 1 doesn't stretch to
	// the button's natural (padded) width and open a gap before the dec column.
	btn_radix->setFixedWidth(num_w);

	for (int row=0 ; row<n ; row++) {
		QLabel *name = new QLabel(rows[row].name, this);
		name->setFont(mono);
		grid->addWidget(name, row + 1, 0);

		num[row] = new QLineEdit(this);
		num[row]->setFont(mono);
		num[row]->setAlignment(Qt::AlignRight);
		num[row]->setFixedSize(num_w, field_h);
		num[row]->setTextMargins(2, 0, 2, 0);
		grid->addWidget(num[row], row + 1, 1);
		connect(num[row], &QLineEdit::editingFinished, this, [=](){ commit_num(row); });

		if (rows[row].has_dec) {
			dec[row] = new QLineEdit(this);
			dec[row]->setFont(mono);
			dec[row]->setAlignment(Qt::AlignRight);
			dec[row]->setFixedSize(dec_w, field_h);
			dec[row]->setTextMargins(2, 0, 2, 0);
			grid->addWidget(dec[row], row + 1, 2);
			connect(dec[row], &QLineEdit::editingFinished, this, [=](){ commit_dec(row); });

			ghost[row] = new QLabel(this);
			ghost[row]->setFont(mono);
			// reserve the ghost column width permanently so the module doesn't
			// narrow when no value is negative and then jump wider when one is.
			ghost[row]->setFixedWidth(ghost_w);
			grid->addWidget(ghost[row], row + 1, 3);
		} else {
			dec[row] = nullptr;
			ghost[row] = nullptr;
		}
	}

	grid->setColumnStretch(4, 1);
	grid->setRowStretch(n + 1, 1);

	apply_ghost_color();

	for (int row=0 ; row<n ; row++) {
		cur[row] = e->get_reg(rows[row].reg_id);
		render_row(row);
	}

	connect(e, &EmuModel::signal_reg_changed, this, &RegCompact::slot_reg_changed);
}

// -----------------------------------------------------------------------
// Paint the ghost labels with the theme's dimmed-but-readable text color.
// Resolving this from the live palette (rather than a "color: palette(mid)"
// stylesheet) avoids the stylesheet's one-shot polish-time resolution, which
// latched onto the startup palette and left the ghost faint until the first
// theme switch forced a re-polish.
void RegCompact::apply_ghost_color()
{
	QColor c = em400_dim_text_color(palette());
	for (QLabel *g : ghost) {
		if (!g) continue;
		QPalette pal = g->palette();
		pal.setColor(QPalette::WindowText, c);
		g->setPalette(pal);
	}
}

// -----------------------------------------------------------------------
// Re-derive the ghost color whenever the application palette/style changes
// (theme toggle), since it is computed from the live palette.
void RegCompact::changeEvent(QEvent *ev)
{
	QWidget::changeEvent(ev);
	if (ev->type() == QEvent::PaletteChange || ev->type() == QEvent::StyleChange) {
		apply_ghost_color();
	}
}

// -----------------------------------------------------------------------
// KB is the control-panel keys register and has its own setter path.
void RegCompact::set_reg_value(int reg_id, uint16_t v)
{
	if (reg_id == EM400_REG_KB) e->set_kb(v);
	else e->set_reg(reg_id, v);
}

// -----------------------------------------------------------------------
// force=true re-renders even a focused field: used right after a commit so the
// just-edited value canonicalises (e.g. "-0" -> "0", "60000" -> "-5536"); the
// focus guard otherwise protects in-progress typing from async reg updates.
void RegCompact::render_row(int row, bool force)
{
	uint16_t v = cur[row];

	if (force || !num[row]->hasFocus()) {
		if (radix == 16) num[row]->setText(QString("%1").arg(v, 4, 16, QChar('0')));
		else num[row]->setText(QString("%1").arg(v, 6, 8, QChar('0')));
	}

	if (!dec[row]) return;

	// decimal: signed value in the editable field; when the top bit is set the
	// unsigned reading diverges, shown as a dimmed ghost beside it.
	if (force || !dec[row]->hasFocus()) {
		dec[row]->setText(QString::number((int16_t)v));
	}
	ghost[row]->setText(v & 0x8000 ? QString::number(v) : QString());
}

// -----------------------------------------------------------------------
void RegCompact::commit_num(int row)
{
	QString s = num[row]->text().trimmed();
	bool ok = false;
	uint val;
	if (radix == 16) {
		if (s.startsWith("0x", Qt::CaseInsensitive)) s = s.mid(2);
		val = s.toUInt(&ok, 16);
	} else {
		val = s.toUInt(&ok, 8);
	}
	if (ok) {
		cur[row] = val & 0xffff;
		set_reg_value(rows[row].reg_id, cur[row]);
	}
	render_row(row, true);
}

// -----------------------------------------------------------------------
void RegCompact::commit_dec(int row)
{
	QString s = dec[row]->text().trimmed();
	bool ok = false;
	uint16_t v = cur[row];
	// a leading '-' means signed; otherwise unsigned (which also covers the
	// > 32767 range that signed could not represent).
	if (s.startsWith('-')) {
		v = (uint16_t)s.toInt(&ok, 10);
	} else {
		v = (uint16_t)(s.toUInt(&ok, 10) & 0xffff);
	}
	if (ok) {
		cur[row] = v;
		set_reg_value(rows[row].reg_id, cur[row]);
	}
	render_row(row, true);
}

// -----------------------------------------------------------------------
void RegCompact::toggle_radix()
{
	radix = (radix == 16) ? 8 : 16;
	btn_radix->setText(radix == 16 ? "hex" : "oct");
	for (int row=0 ; row<rows.size() ; row++) render_row(row);
}

// -----------------------------------------------------------------------
void RegCompact::slot_reg_changed(int reg, uint16_t val)
{
	for (int row=0 ; row<rows.size() ; row++) {
		if (rows[row].reg_id != reg) continue;
		if (cur[row] == val) return;
		cur[row] = val;
		render_row(row);
		return;
	}
}

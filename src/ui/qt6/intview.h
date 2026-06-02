#ifndef INTVIEW_H
#define INTVIEW_H

#include <QWidget>
#include <QFrame>
#include <QVector>
#include <QLabel>
#include <QPushButton>
#include <QFont>
#include "emumodel.h"

// -----------------------------------------------------------------------
// Clickable bordered box wrapping the interrupts that share one RM mask bit.
// The border reflects the mask state (lit = allowed, dimmed = blocked); a
// click on the box itself (its padding/border, not on a child cell) toggles
// that mask bit in SR.
// -----------------------------------------------------------------------
class MaskBox : public QFrame {
	Q_OBJECT

public:
	explicit MaskBox(int sr_bit, QWidget *parent = nullptr);
	int sr_bit; // SR bit (6-15) that gates this group

signals:
	void clicked();

protected:
	void mousePressEvent(QMouseEvent *ev) override;
};

// -----------------------------------------------------------------------
// RZ / interrupt-mask decode widget. The 32 interrupt bits are laid out in
// four semantic rows (hardware / software / I/O channels / other); cells that
// share an RM mask bit are wrapped in a clickable MaskBox.
//
//   - left click  -> toggle the interrupt (RZ bit), via em400_int_set/clear;
//                    works whether the CPU runs or is stopped.
//   - right click -> toggle the mask (SR RM bit), via a register write; only
//                    takes effect while the CPU is STOPPED (the reg-edit path).
//
// Three states are rendered per cell from RZ + SR alone (no emulation-internal
// xmask needed): pending = RZ bit set; unmasked = the gating SR bit set (or the
// interrupt is non-maskable); the will-fire case (pending AND unmasked) gets a
// distinct highlight. All labels/tooltips live here - the library deals in
// structure (em400_int_mask_bit), not language.
// -----------------------------------------------------------------------
class IntView : public QWidget {
	Q_OBJECT

public:
	explicit IntView(EmuModel *emu, QWidget *parent = nullptr);

protected:
	void changeEvent(QEvent *ev) override;
	bool eventFilter(QObject *obj, QEvent *ev) override;

private slots:
	void slot_rz_changed(uint32_t val);
	void slot_reg_changed(int reg, uint16_t val);

private:
	EmuModel *e;
	QFont fnt;       // row labels + RZ header
	QFont cell_fnt;  // interrupt cells (a point smaller, to set them apart)
	uint32_t rz = 0;
	uint16_t sr = 0;
	int mask_bit[32];           // SR bit gating each interrupt, -1 if non-maskable
	QPushButton *cell[32] = {0};
	QVector<MaskBox*> boxes;
	QLabel *help = nullptr; // "?" badge (top-right)

	QPushButton * make_cell(int n);
	void toggle_mask(int sr_bit);
	void render_cell(int n);
	void render_box(MaskBox *b);
	void render_help();
	void render_all();
};

#endif // INTVIEW_H

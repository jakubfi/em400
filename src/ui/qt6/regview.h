#ifndef REGVIEW_H
#define REGVIEW_H

#include <QWidget>
#include <QVector>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include "emumodel.h"

class QEvent;

// -----------------------------------------------------------------------
// Compact register table, built from a row descriptor so the same widget
// serves both user and system registers.
//
// Column 1 is the value in hex, toggled to octal by a single header button
// (all rows switch together; the toggle is independent per widget instance).
// Column 2 is decimal and only present for "data" registers: a single number
// for 0..32767 (signed and unsigned coincide), and when the top bit is set it
// shows the signed value plus a dimmed unsigned "ghost". Editing the decimal
// field self-disambiguates: a leading '-' or a value > 32767 can only be one
// interpretation.
//
//   - USER   (R0-R7): every row is data (hex/oct + decimal).
//   - SYSTEM (IC, AR, AC, IR, KB, SR): address/opaque registers carry only the
//     hex/oct field (the row simply ends there - no blank cell); the data
//     registers AC and KB also get the decimal column. RZ is intentionally
//     absent - it is a 32-bit interrupt-request bitfield with no meaningful
//     numeric reading and belongs in a dedicated bit-decode view.
// -----------------------------------------------------------------------
class RegCompact : public QWidget {
	Q_OBJECT

public:
	enum Kind { USER, SYSTEM };
	explicit RegCompact(EmuModel *emu, Kind kind, QWidget *parent = nullptr);

private slots:
	void slot_reg_changed(int reg, uint16_t val);
	void toggle_radix();

private:
	struct Row {
		int reg_id;      // EM400_REG_*
		QString name;
		bool has_dec;    // true: also show the decimal column
	};

	EmuModel *e;
	int radix = 16;                    // 16 (hex) or 8 (oct)
	QPushButton *btn_radix;
	QVector<Row> rows;
	QVector<QLineEdit*> num;            // hex/oct field (every row)
	QVector<QLineEdit*> dec;            // signed-decimal field (data rows, else nullptr)
	QVector<QLabel*> ghost;             // dimmed unsigned value (data rows, else nullptr)
	QVector<uint16_t> cur;

	void set_reg_value(int reg_id, uint16_t v);
	void commit_num(int row);
	void commit_dec(int row);
	void render_row(int row, bool force = false);
	void apply_ghost_color();

protected:
	void changeEvent(QEvent *ev) override;
};

#endif // REGVIEW_H

// vim: tabstop=4 shiftwidth=4 autoindent

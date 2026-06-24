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
// Every row shows three value columns:
//   - HEX/OCT: the raw value, toggled between hex and octal by a single header
//     button (all rows switch together; the toggle is per widget instance).
//   - DEC:  editable signed reading.
//   - UDEC: read-only unsigned reading, dimmed.
// Editing the dec field self-disambiguates: a leading '-' or a value > 32767
// can only be one interpretation.
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
	};

	EmuModel *e;
	int radix = 16;                    // 16 (hex) or 8 (oct)
	QPushButton *btn_radix;
	QVector<Row> rows;
	QVector<QLineEdit*> num;            // hex/oct field (every row)
	QVector<QLineEdit*> dec;            // signed-decimal field (data rows, else nullptr)
	QVector<QLineEdit*> udec;          // dimmed unsigned value, read-only (data rows, else nullptr)
	QVector<uint16_t> cur;

	void set_reg_value(int reg_id, uint16_t v);
	void commit_num(int row);
	void commit_dec(int row);
	void render_row(int row, bool force = false);
	void apply_udec_color();

protected:
	void changeEvent(QEvent *ev) override;
};

#endif // REGVIEW_H

// vim: tabstop=4 shiftwidth=4 autoindent

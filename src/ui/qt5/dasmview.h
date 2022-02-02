#ifndef DASMVIEW_H
#define DASMVIEW_H

#include <QWidget>
#include "emumodel.h"

// -----------------------------------------------------------------------
class AsmLine {

public:
	int nb;
	int addr;
	int length;
	QString text;
	AsmLine(int nb, int addr, int length, const char *text);
	AsmLine(int nb, int addr, int length, QString text);
	~AsmLine();
};

// -----------------------------------------------------------------------
class DasmView : public QWidget {

	Q_OBJECT

public:
	explicit DasmView(QWidget *parent = nullptr);
	~DasmView();
	void connect_emu(EmuModel *emu) { e = emu; }

public slots:
	void update_contents(int nb, int addr);

private:
	int dasm_total_lines;
	int bottom, right;
	int context;
	int offset;
	int half_font_width;
	int line_height;
	int interline;
	int addr_x_start, addr_y_start;
	int addr_len; // characters
	int dasm_x_start, dasm_y_start;
	int divider_x_pos;

	int nb, addr;

	EmuModel *e;
	struct emdas *emd;
	QList<AsmLine> listing;
	QFont font;
	int font_height, font_width, font_descent;

	void set_font(QString name, int size=0);
	void update_font_related_dimensions();
	void internal_update_contents();

	AsmLine dasm_exact(int nb, int addr);
	AsmLine dasm_fuzzy(int nb, int addr, int fuzziness=5);
	int prepend(int i);
	int append(int i);
	int delete_last(int i);
	int delete_first(int i);

protected:
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);
	void wheelEvent(QWheelEvent *event);

};

#endif // DASMVIEW_H

#ifndef DASMVIEW_H
#define DASMVIEW_H

#include <QWidget>
#include <QScrollBar>
#include "emumodel.h"
#include "asmline.h"

// -----------------------------------------------------------------------
class DasmView : public QWidget {

	Q_OBJECT

public:
	explicit DasmView(QWidget *parent = nullptr);
	~DasmView();
	void connect_emu(EmuModel *emu) { e = emu; }

public slots:
	void update_contents(int nb, int addr);
	void update_contents_no_nb(int new_addr);

private:
	EmuModel *e;

	int cnb, caddr;

	QFont font;
	int font_height, font_width, font_descent; // calculated

	const int fuzziness = 5;			// fuzziness when searching for instruction start address
	const int dasm_line_length = 26;	// length of disassembly line
	int dasm_total_lines = 0;			// lines of disassembly visible
	int line_height;					// disassembly line height in pixels, with space between lines
	const int interline = 4;			// additional interline, in pixels
	const int addr_len = 4;				// address length, in characters

	QScrollBar *scroll;

	struct emdas *emd;
	QList<AsmLine> listing;

	void set_font(QString name, int size=0);
	void internal_update_contents();

	AsmLine dasm_exact(int nb, int addr);
	AsmLine dasm_fuzzy(int nb, int addr);
	int prepend(int i, QList<AsmLine>& l);
	int append(int i, QList<AsmLine>& l);
	int delete_last(int i);
	int delete_first(int i);

protected:
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);
	void wheelEvent(QWheelEvent *event);
	QSize minimumSizeHint() const;
	QSize sizeHint() const;
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);

};

#endif // DASMVIEW_H

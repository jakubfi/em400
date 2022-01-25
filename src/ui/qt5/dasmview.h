#ifndef DASMVIEW_H
#define DASMVIEW_H

#include <QWidget>
#include "emumodel.h"

class DasmView : public QWidget
{
	Q_OBJECT

public:
	explicit DasmView(QWidget *parent = nullptr);
	~DasmView();
	void connect_emu(EmuModel *emu) { e = emu; }

private:
	EmuModel *e;
	struct emdas *emd;

	QString dasm(uint16_t addr, int *words);

protected:
	void paintEvent(QPaintEvent *event);

};

#endif // DASMVIEW_H

#include <QPainter>
#include <QPaintEvent>
#include <emdas.h>
#include "dasmview.h"


// -----------------------------------------------------------------------
int dbg_mem_get(int nb, uint16_t addr, uint16_t *data)
{
	return ectl_mem_get(nb, addr, data, 1);
}

// -----------------------------------------------------------------------
DasmView::DasmView(QWidget *parent) : QWidget(parent)
{
	emd = emdas_create(EMD_ISET_MX16, dbg_mem_get);
	emdas_set_nl(emd, '\0');
	emdas_set_features(emd, EMD_FEAT_NONE);
	emdas_set_tabs(emd, 0, 0, 4, 4);
}

// -----------------------------------------------------------------------
DasmView::~DasmView()
{
	emdas_destroy(emd);
}

// -----------------------------------------------------------------------
QString DasmView::dasm(uint16_t addr, int *words)
{
	*words = emdas_dasm(emd, ectl_reg_get(ECTL_REG_Q) * ectl_reg_get(ECTL_REG_NB), addr);
	char *buf = emdas_get_buf(emd);
	return QString("%1:  %2").arg(addr, 4, 16, QLatin1Char('0')).arg(buf);
}

// -----------------------------------------------------------------------
void DasmView::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.fillRect(event->rect(), this->palette().color(QPalette::Base));
	QFont f("Monospace");
	painter.setFont(f);
	int words;
	painter.drawText(0, 30, dasm(e->get_reg(ECTL_REG_IC), &words));
}


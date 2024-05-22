//  Copyright (c) 2022 Jakub Filipowicz <jakubf@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include <QApplication>

#include "mainwindow.h"

#include "ui/ui.h"

struct ui_qt5_data {
};

// -----------------------------------------------------------------------
void * ui_qt5_setup(const char *call_name)
{
	struct ui_qt5_data *ui = (struct ui_qt5_data *) calloc(1, sizeof(struct ui_qt5_data));
	if (!ui) {
		return NULL;
	}

	return ui;
}

// -----------------------------------------------------------------------
void ui_qt5_loop(void *data)
{
	int argv = 1;
	char *argc[] = { (char*)"em400" };
	QApplication a(argv, argc);
	MainWindow w;

	w.show();
	a.exec();
}

// -----------------------------------------------------------------------
void ui_qt5_stop(void *data)
{

}

// -----------------------------------------------------------------------
void ui_qt5_destroy(void *data)
{
	struct ui_qt5_data *ui = (struct ui_qt5_data *) data;
	free(ui);
}

// -----------------------------------------------------------------------
struct ui_drv ui_qt5 = {
	.name = "qt5",
	.setup = ui_qt5_setup,
	.loop = ui_qt5_loop,
	.stop = ui_qt5_stop,
	.destroy = ui_qt5_destroy
};

// vim: tabstop=4 shiftwidth=4 autoindent

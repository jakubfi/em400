//  Copyright (c) 2022-2026 Jakub Filipowicz <jakubf@gmail.com>
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
#include <QSettings>
#include <QTranslator>
#include <QLocale>

#include "mainwindow.h"
#include "theme.h"

#include "ui/ui.h"
#include "em400.h"

struct ui_qt6_data {
};

// -----------------------------------------------------------------------
void * ui_qt6_setup(const char *call_name)
{
	struct ui_qt6_data *ui = (struct ui_qt6_data *) calloc(1, sizeof(struct ui_qt6_data));
	if (!ui) {
		return NULL;
	}

	return ui;
}

// -----------------------------------------------------------------------
void ui_qt6_loop(void *data)
{
	int argv = 1;
	char *argc[] = { (char*)"em400" };
	QApplication a(argv, argc);
	QApplication::setOrganizationName("em400");
	QApplication::setApplicationName("em400-qt");

	QTranslator *translator = new QTranslator(&a);
	if (translator->load(QLocale::system(), "em400-qt", "_", ":/i18n")) {
		a.installTranslator(translator);
	}

	// default custom dark theme
	QSettings settings;
	em400_apply_theme(settings.value("ui/panelTheme", true).toBool());

	if (settings.value("ui/startPoweredOn", false).toBool()) {
		em400_power_on();
	}

	MainWindow w;

	w.show();
	a.exec();
}

// -----------------------------------------------------------------------
void ui_qt6_destroy(void *data)
{
	struct ui_qt6_data *ui = (struct ui_qt6_data *) data;
	free(ui);
}

// -----------------------------------------------------------------------
struct ui_drv ui_qt6 = {
	.name = "qt",
	.setup = ui_qt6_setup,
	.loop = ui_qt6_loop,
	.destroy = ui_qt6_destroy,
	.deferred_power = true
};

// vim: tabstop=4 shiftwidth=4 autoindent

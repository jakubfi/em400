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
#include <QSettings>
#include <QTranslator>
#include <QLocale>

#include "mainwindow.h"
#include "theme.h"

#include "ui/ui.h"

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
	// identity for QSettings (window/dock layout persistence). "em400-qt" keeps the
	// UI's settings namespace distinct from the future em400 core library config.
	QApplication::setOrganizationName("em400");
	QApplication::setApplicationName("em400-qt");

	// Localize the UI to the system locale. English is the built-in source
	// language: for a C/English locale load() finds no catalog and returns false,
	// leaving the original strings in place. A pl_* locale loads em400-qt_pl.qm
	// embedded under :/i18n.
	QTranslator *translator = new QTranslator(&a);
	if (translator->load(QLocale::system(), "em400-qt", "_", ":/i18n")) {
		a.installTranslator(translator);
	}

	// Force the dark control-panel theme by default; honor a persisted choice
	// if the user has previously turned it off via View > Panel Theme.
	QSettings settings;
	em400_apply_theme(settings.value("ui/panelTheme", true).toBool());

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
	.destroy = ui_qt6_destroy
};

// vim: tabstop=4 shiftwidth=4 autoindent

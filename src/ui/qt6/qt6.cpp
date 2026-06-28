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
#include "libem400.h"
#include "appcfg.h"
#include "app_err.h"

struct ui_qt6_data {
};

static const char *ORG_NAME = "em400";
static const char *APP_NAME = "em400-qt";

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
static int ui_qt6_poweron(void *data, const char *program)
{
	QSettings settings(ORG_NAME, APP_NAME);
	if (!program && !settings.value("ui/startPoweredOn", false).toBool()) {
		return E_OK;
	}
	int res = em400_init(appcfg_active_machine(&appcfg), &appcfg.host);
	app_msg_drain();
	if (res != E_OK) {
		return E_ERR;
	}
	if (program && !em400_load_os_image_path(program)) {
		return app_err("Preloading OS memory failed: %s", program);
	}
	return E_OK;
}

// -----------------------------------------------------------------------
void ui_qt6_loop(void *data)
{
	int argv = 1;
	char *argc[] = { (char*)"em400" };
	QApplication a(argv, argc);
	QApplication::setOrganizationName(ORG_NAME);
	QApplication::setApplicationName(APP_NAME);

	QTranslator *translator = new QTranslator(&a);
	if (translator->load(QLocale::system(), "em400-qt", "_", ":/i18n")) {
		a.installTranslator(translator);
	}

	// default custom dark theme
	QSettings settings;
	em400_apply_theme(settings.value("ui/panelTheme", true).toBool());

	MainWindow w;

	w.show();
	a.exec();
}

// -----------------------------------------------------------------------
static void ui_qt6_poweroff(void *data)
{
	em400_shutdown();
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
	.poweron = ui_qt6_poweron,
	.loop = ui_qt6_loop,
	.poweroff = ui_qt6_poweroff,
	.destroy = ui_qt6_destroy,
};

// vim: tabstop=4 shiftwidth=4 autoindent

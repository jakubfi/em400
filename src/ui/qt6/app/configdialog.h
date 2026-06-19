//  Copyright (c) 2026 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QVector>

#include "appcfg.h"

class QStackedWidget;
class QListWidget;
class QCheckBox;
class QSpinBox;

// Edits `appcfg` in place; no apply/restart and no file write yet (later steps).
class ConfigDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ConfigDialog(QWidget *parent = nullptr);

private:
	QListWidget *sections;
	QStackedWidget *stack;

	struct appcfg_machine *machine = nullptr;

	QComboBox *m_active = nullptr;
	QLineEdit *m_name = nullptr;
	QCheckBox *m_awp = nullptr, *m_mod = nullptr, *m_user_io_illegal = nullptr, *m_nomem_stop = nullptr;
	QComboBox *m_clock_period = nullptr;
	QSpinBox *m_elwro = nullptr, *m_mega = nullptr, *m_os_segments = nullptr;
	QLineEdit *m_mega_prom = nullptr, *m_preload = nullptr;

	QVector<QCheckBox *> m_log_components;

	QWidget *build_general_page();
	QWidget *build_sound_page();
	QWidget *build_machine_page();
	QWidget *build_log_page();

	void add_section(const QString &title, const QString &icon_name, QWidget *page);
	void reload_machine_page();
	void rebuild_log_components();

private slots:
	void slot_active_machine_changed(int index);
};

#endif // CONFIGDIALOG_H

// vim: tabstop=4 shiftwidth=4 autoindent

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
class QTreeWidget;
class QPushButton;
class ConfigController;

// Edits a private working copy of `appcfg`; OK commits it (write file, apply live
// media + volume, overwrite appcfg), Cancel discards it. Cold edits materialize at
// the next power-on; removable-media swaps apply live on OK.
class ConfigDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ConfigDialog(ConfigController *ctl, QWidget *parent = nullptr);
	~ConfigDialog() override;

	// restores the live volume preview before discarding the working copy
	void reject() override;

private:
	ConfigController *ctl;
	QListWidget *sections;
	QStackedWidget *stack;

	// working copy: all edits land here and are committed to appcfg only on OK
	struct appcfg work {};
	struct appcfg_machine *machine = nullptr; // points into `work`
	int orig_volume = 0; // live buzzer volume at open, restored on Cancel

	// mono font (QSettings) captured at open
	// the font applies live, so Cancel restores this state.
	bool orig_mono_font_set = false;
	QString orig_mono_font_family;
	int orig_mono_font_size = 0;
	bool mono_font_touched = false; // any Change/Reset happened this session

	QWidget *machine_page = nullptr;

	QComboBox *m_active = nullptr;
	QLineEdit *m_name = nullptr;
	QCheckBox *m_awp = nullptr, *m_mod = nullptr, *m_user_io_illegal = nullptr, *m_nomem_stop = nullptr;
	QComboBox *m_clock_period = nullptr;
	QSpinBox *m_elwro = nullptr, *m_mega = nullptr, *m_os_segments = nullptr;
	QLineEdit *m_mega_prom = nullptr, *m_preload = nullptr;

	QVector<QCheckBox *> m_log_components;

	QTreeWidget *io_tree = nullptr;
	QStackedWidget *io_editor = nullptr;
	QComboBox *io_chan_num = nullptr, *io_chan_type = nullptr;
	QComboBox *io_dev_num = nullptr, *io_dev_type = nullptr;
	QWidget *io_dev_params = nullptr;
	QPushButton *io_add_dev_btn = nullptr, *io_remove_btn = nullptr, *io_add_chan_btn = nullptr;
	int io_sel_chan = -1;
	int io_sel_dev = -1;

	QWidget *build_general_page();
	QWidget *build_sound_page();
	QWidget *build_machine_page();
	QWidget *build_io_page();
	QWidget *build_log_page();

	void add_section(const QString &title, const QString &icon_name, QWidget *page);
	void reload_machine_page();
	void rebuild_log_components();

	QString chan_type_label(int type);
	QString dev_type_label(int type);
	void io_build_tree();
	void io_selection_changed();
	void io_rebuild_dev_params();
	void io_update_buttons();
	void io_add_channel();
	void io_add_device();
	void io_remove_selected();
	void io_set_channel_number(int num);
	void io_set_device_number(int num);

public slots:
	// re-eval gating; connected to EmuModel::signal_power_changed so flipping the
	// ignition while the (non-modal) dialog is open updates cold-field enabledness
	void update_enabled_states();

	// a Devices-menu swap landed while we are open: mirror it into the working copy
	// (menu wins over any pending edit to the same slot) so OK can't commit a stale
	// value, and refresh the field if that device is on screen
	void on_media_changed(unsigned chan, unsigned dev, unsigned slot, QString path);

signals:
	void signal_machine_renamed();
	void signal_gui_volume_changed(int volume);
	void signal_mono_font_changed();

private slots:
	void slot_active_machine_changed(int index);
	void accept_config();
};

#endif // CONFIGDIALOG_H

// vim: tabstop=4 shiftwidth=4 autoindent

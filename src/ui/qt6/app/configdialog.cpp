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

#include <cstdlib>
#include <cstring>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QIcon>
#include <QStackedWidget>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QTabWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QSignalBlocker>
#include <QStringList>
#include <QSet>

#include "configdialog.h"

// -----------------------------------------------------------------------
// strdup/free, not new/delete: appcfg_free() releases these with free().
namespace {
void set_cstr(const char **field, const QString &s)
{
	free((void *) *field);
	*field = s.isEmpty() ? nullptr : strdup(s.toUtf8().constData());
}
}

// -----------------------------------------------------------------------
ConfigDialog::ConfigDialog(QWidget *parent) :
	QDialog(parent)
{
	setWindowTitle(tr("Configuration"));

	machine = appcfg_machine_find(&appcfg, appcfg.active_id);
	if (!machine && appcfg.n_machines) {
		machine = &appcfg.machines[0];
	}

	sections = new QListWidget();
	sections->setMaximumWidth(170);
	sections->setIconSize(QSize(24, 24));
	stack = new QStackedWidget();

	add_section(tr("Machine"), "computer", build_machine_page());
	add_section(tr("General"), "preferences-system", build_general_page());
	add_section(tr("Sound"), "audio-volume-high", build_sound_page());
	add_section(tr("Logging"), "text-x-generic", build_log_page());

	reload_machine_page();

	connect(sections, &QListWidget::currentRowChanged, stack, &QStackedWidget::setCurrentIndex);
	sections->setCurrentRow(0);

	QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::accept);

	QHBoxLayout *body = new QHBoxLayout();
	body->addWidget(sections);
	body->addWidget(stack, 1);

	QVBoxLayout *outer = new QVBoxLayout(this);
	outer->addLayout(body);
	outer->addWidget(buttons);

	setMinimumSize(460, 460);
}

// -----------------------------------------------------------------------
void ConfigDialog::add_section(const QString &title, const QString &icon_name, QWidget *page)
{
	QListWidgetItem *item = new QListWidgetItem(QIcon::fromTheme(icon_name), title, sections);
	item->setSizeHint(QSize(150, 38));
	stack->addWidget(page);
}

// -----------------------------------------------------------------------
QWidget *ConfigDialog::build_general_page()
{
	QWidget *page = new QWidget();
	QFormLayout *form = new QFormLayout(page);
	form->setVerticalSpacing(10);

	QComboBox *ui = new QComboBox();
	ui->addItem(tr("Curses (text debugger)"), "curses");
	ui->addItem(tr("Qt (graphical)"), "qt");
	ui->addItem(tr("Command line"), "cmd");
	int ui_idx = ui->findData(appcfg.ui ? QString(appcfg.ui) : QString());
	if (ui_idx >= 0) ui->setCurrentIndex(ui_idx);
	connect(ui, &QComboBox::currentIndexChanged, this, [ui]() {
		set_cstr((const char **) &appcfg.ui, ui->currentData().toString());
	});
	form->addRow(tr("User interface:"), ui);

	QCheckBox *speed_real = new QCheckBox(tr("Emulate real CPU speed"));
	speed_real->setChecked(appcfg.host.emu.speed_real);
	connect(speed_real, &QCheckBox::toggled, this, [](bool on) {
		appcfg.host.emu.speed_real = on;
	});
	form->addRow(QString(), speed_real);

	QSpinBox *quantum = new QSpinBox();
	quantum->setRange(50, 900);
	quantum->setSingleStep(10);
	quantum->setSuffix(tr(" us"));
	quantum->setValue(appcfg.host.emu.emulation_quantum_us);
	connect(quantum, &QSpinBox::valueChanged, this, [](int v) {
		appcfg.host.emu.emulation_quantum_us = v;
	});
	form->addRow(tr("Emulation quantum:"), quantum);

	return page;
}

// -----------------------------------------------------------------------
QWidget *ConfigDialog::build_sound_page()
{
	QWidget *page = new QWidget();
	QFormLayout *form = new QFormLayout(page);
	form->setVerticalSpacing(10);

	QCheckBox *enabled = new QCheckBox(tr("Sound output enabled"));
	enabled->setChecked(appcfg.host.sound.enabled);
	connect(enabled, &QCheckBox::toggled, this, [](bool on) {
		appcfg.host.sound.enabled = on;
	});
	form->addRow(QString(), enabled);

	QSlider *volume = new QSlider(Qt::Horizontal);
	volume->setRange(0, 100);
	volume->setValue(appcfg.host.sound.volume);
	QLabel *volume_val = new QLabel(QString::number(appcfg.host.sound.volume));
	volume_val->setMinimumWidth(volume_val->fontMetrics().horizontalAdvance("100"));
	connect(volume, &QSlider::valueChanged, this, [volume_val](int v) {
		appcfg.host.sound.volume = v;
		volume_val->setText(QString::number(v));
	});
	QHBoxLayout *volume_row = new QHBoxLayout();
	volume_row->addWidget(volume, 1);
	volume_row->addWidget(volume_val);
	form->addRow(tr("Volume:"), volume_row);

	QComboBox *rate = new QComboBox();
	for (int r : {8000, 11025, 16000, 22050, 32000, 44100, 48000, 88200, 96000, 192000}) {
		rate->addItem(tr("%1 Hz").arg(r), r);
	}
	if (rate->findData(appcfg.host.sound.sample_rate) < 0) {
		rate->addItem(tr("%1 Hz").arg(appcfg.host.sound.sample_rate), appcfg.host.sound.sample_rate);
	}
	rate->setCurrentIndex(rate->findData(appcfg.host.sound.sample_rate));
	connect(rate, &QComboBox::currentIndexChanged, this, [rate]() {
		appcfg.host.sound.sample_rate = rate->currentData().toInt();
	});
	form->addRow(tr("Sample rate:"), rate);

	QComboBox *buffer = new QComboBox();
	for (int b=16 ; b<=8192 ; b*=2) {
		buffer->addItem(QString::number(b), b);
	}
	if (buffer->findData(appcfg.host.sound.buffer_len) < 0) {
		buffer->addItem(QString::number(appcfg.host.sound.buffer_len), appcfg.host.sound.buffer_len);
	}
	buffer->setCurrentIndex(buffer->findData(appcfg.host.sound.buffer_len));
	connect(buffer, &QComboBox::currentIndexChanged, this, [buffer]() {
		appcfg.host.sound.buffer_len = buffer->currentData().toInt();
	});
	form->addRow(tr("Buffer length (frames):"), buffer);

	QSpinBox *latency = new QSpinBox();
	latency->setRange(0, 1000);
	latency->setSuffix(tr(" ms"));
	latency->setValue(appcfg.host.sound.latency);
	connect(latency, &QSpinBox::valueChanged, this, [](int v) {
		appcfg.host.sound.latency = v;
	});
	form->addRow(tr("Latency:"), latency);

	QLineEdit *backend = new QLineEdit();
	backend->setText(appcfg.host.sound.backend ? QString(appcfg.host.sound.backend) : QString());
	connect(backend, &QLineEdit::editingFinished, this, [backend]() {
		set_cstr(&appcfg.host.sound.backend, backend->text());
	});
	form->addRow(tr("Backend:"), backend);

	QLineEdit *device = new QLineEdit();
	device->setText(appcfg.host.sound.device ? QString(appcfg.host.sound.device) : QString());
	connect(device, &QLineEdit::editingFinished, this, [device]() {
		set_cstr(&appcfg.host.sound.device, device->text());
	});
	form->addRow(tr("Device:"), device);

	return page;
}

// -----------------------------------------------------------------------
QWidget *ConfigDialog::build_machine_page()
{
	QWidget *page = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout(page);

	QFormLayout *id_form = new QFormLayout();
	id_form->setVerticalSpacing(10);

	m_active = new QComboBox();
	for (int i=0 ; i<appcfg.n_machines ; i++) {
		const struct appcfg_machine *m = &appcfg.machines[i];
		m_active->addItem(m->name ? QString(m->name) : QString(m->id), QString(m->id));
	}
	int act_idx = m_active->findData(appcfg.active_id ? QString(appcfg.active_id) : QString());
	if (act_idx >= 0) m_active->setCurrentIndex(act_idx);
	connect(m_active, &QComboBox::currentIndexChanged, this, &ConfigDialog::slot_active_machine_changed);
	id_form->addRow(tr("Active machine:"), m_active);

	m_name = new QLineEdit();
	connect(m_name, &QLineEdit::editingFinished, this, [this]() {
		if (!machine) return;
		appcfg_machine_set_name(machine, m_name->text().toUtf8().constData());
		m_active->setItemText(m_active->currentIndex(),
			machine->name ? QString(machine->name) : QString(machine->id));
	});
	id_form->addRow(tr("Name:"), m_name);
	layout->addLayout(id_form);

	QTabWidget *tabs = new QTabWidget();

	QWidget *cpu_box = new QWidget();
	QFormLayout *cpu = new QFormLayout(cpu_box);
	cpu->setVerticalSpacing(10);
	m_awp = new QCheckBox(tr("Floating point unit (AWP)"));
	connect(m_awp, &QCheckBox::toggled, this, [this](bool on) {
		if (machine) machine->cfg.cpu.awp = on;
	});
	cpu->addRow(QString(), m_awp);
	m_mod = new QCheckBox(tr("CPU modifications"));
	connect(m_mod, &QCheckBox::toggled, this, [this](bool on) {
		if (machine) machine->cfg.cpu.mod = on;
	});
	cpu->addRow(QString(), m_mod);
	m_user_io_illegal = new QCheckBox(tr("I/O illegal in user mode"));
	connect(m_user_io_illegal, &QCheckBox::toggled, this, [this](bool on) {
		if (machine) machine->cfg.cpu.user_io_illegal = on;
	});
	cpu->addRow(QString(), m_user_io_illegal);
	m_nomem_stop = new QCheckBox(tr("Stop on missing memory"));
	connect(m_nomem_stop, &QCheckBox::toggled, this, [this](bool on) {
		if (machine) machine->cfg.cpu.nomem_stop = on;
	});
	cpu->addRow(QString(), m_nomem_stop);
	m_clock_period = new QComboBox();
	for (int ms : {2, 4, 8, 10, 20}) {
		m_clock_period->addItem(tr("%1 ms").arg(ms), ms);
	}
	connect(m_clock_period, &QComboBox::currentIndexChanged, this, [this]() {
		if (machine) machine->cfg.cpu.clock_period_ms = m_clock_period->currentData().toInt();
	});
	cpu->addRow(tr("Clock period:"), m_clock_period);
	tabs->addTab(cpu_box, tr("CPU"));

	QWidget *mem_box = new QWidget();
	QFormLayout *mem = new QFormLayout(mem_box);
	mem->setVerticalSpacing(10);
	m_elwro = new QSpinBox();
	m_elwro->setRange(0, 16);
	connect(m_elwro, &QSpinBox::valueChanged, this, [this](int v) {
		if (machine) machine->cfg.mem.elwro_modules = v;
	});
	mem->addRow(tr("Elwro modules:"), m_elwro);
	m_mega = new QSpinBox();
	m_mega->setRange(0, 16);
	connect(m_mega, &QSpinBox::valueChanged, this, [this](int v) {
		if (machine) machine->cfg.mem.mega_modules = v;
	});
	mem->addRow(tr("MEGA modules:"), m_mega);
	m_os_segments = new QSpinBox();
	m_os_segments->setRange(1, 2);
	connect(m_os_segments, &QSpinBox::valueChanged, this, [this](int v) {
		if (machine) machine->cfg.mem.os_segments = v;
	});
	mem->addRow(tr("Hardwired OS segments:"), m_os_segments);

	m_mega_prom = new QLineEdit();
	connect(m_mega_prom, &QLineEdit::editingFinished, this, [this]() {
		if (machine) set_cstr(&machine->cfg.mem.mega_prom_image, m_mega_prom->text());
	});
	QPushButton *prom_browse = new QPushButton(tr("Browse..."));
	connect(prom_browse, &QPushButton::clicked, this, [this]() {
		QString f = QFileDialog::getOpenFileName(this, tr("MEGA PROM image"));
		if (!f.isNull()) m_mega_prom->setText(f);
	});
	QHBoxLayout *prom_row = new QHBoxLayout();
	prom_row->addWidget(m_mega_prom, 1);
	prom_row->addWidget(prom_browse);
	mem->addRow(tr("MEGA PROM image:"), prom_row);

	m_preload = new QLineEdit();
	connect(m_preload, &QLineEdit::editingFinished, this, [this]() {
		if (machine) set_cstr(&machine->cfg.mem.preload_image, m_preload->text());
	});
	QPushButton *preload_browse = new QPushButton(tr("Browse..."));
	connect(preload_browse, &QPushButton::clicked, this, [this]() {
		QString f = QFileDialog::getOpenFileName(this, tr("Preload image"));
		if (!f.isNull()) m_preload->setText(f);
	});
	QHBoxLayout *preload_row = new QHBoxLayout();
	preload_row->addWidget(m_preload, 1);
	preload_row->addWidget(preload_browse);
	mem->addRow(tr("Preload image:"), preload_row);
	tabs->addTab(mem_box, tr("Memory"));

	tabs->addTab(new QWidget(), tr("I/O"));

	layout->addWidget(tabs);
	return page;
}

// -----------------------------------------------------------------------
QWidget *ConfigDialog::build_log_page()
{
	QWidget *page = new QWidget();
	QFormLayout *form = new QFormLayout(page);
	form->setVerticalSpacing(10);

	QCheckBox *enabled = new QCheckBox(tr("Logging enabled"));
	enabled->setChecked(appcfg.log.enabled);
	form->addRow(QString(), enabled);

	QWidget *config_box = new QWidget();
	QFormLayout *config_form = new QFormLayout(config_box);
	config_form->setVerticalSpacing(10);
	config_form->setContentsMargins(0, 0, 0, 0);
	config_box->setEnabled(appcfg.log.enabled);
	connect(enabled, &QCheckBox::toggled, this, [config_box](bool on) {
		appcfg.log.enabled = on;
		config_box->setEnabled(on);
	});

	QSet<int> on;
	QString comps = appcfg.log.components ? QString(appcfg.log.components) : QString();
	for (const QString &raw : comps.split(',', Qt::SkipEmptyParts)) {
		QString tok = raw.trimmed();
		if (tok.isEmpty()) continue;
		bool neg = tok.startsWith('-');
		if (neg) tok = tok.mid(1);
		if (tok.compare("all", Qt::CaseInsensitive) == 0) {
			for (int i=1 ; i<L_COUNT ; i++) {
				if (neg) on.remove(i); else on.insert(i);
			}
			continue;
		}
		int id = em400_log_component_id(tok.toUtf8().constData());
		if (id > 0) {
			if (neg) on.remove(id); else on.insert(id);
		}
	}
	on.insert(L_EM4H); // the core always keeps EM4H on

	QGroupBox *comp_box = new QGroupBox(tr("Components"));
	QVBoxLayout *comp_layout = new QVBoxLayout(comp_box);
	QGridLayout *comp_grid = new QGridLayout();
	for (int i=1 ; i<L_COUNT ; i++) {
		QCheckBox *cb = new QCheckBox(QString(em400_log_component_name(i)));
		cb->setProperty("compid", i);
		cb->setChecked(on.contains(i));
		if (i == L_EM4H) cb->setEnabled(false);
		connect(cb, &QCheckBox::toggled, this, [this]() { rebuild_log_components(); });
		comp_grid->addWidget(cb, (i - 1) / 3, (i - 1) % 3);
		m_log_components.append(cb);
	}
	comp_layout->addLayout(comp_grid);

	QHBoxLayout *sel_row = new QHBoxLayout();
	QPushButton *sel_all = new QPushButton(tr("Select all"));
	QPushButton *sel_none = new QPushButton(tr("Select none"));
	connect(sel_all, &QPushButton::clicked, this, [this]() {
		for (QCheckBox *cb : m_log_components) {
			if (cb->isEnabled()) {
				QSignalBlocker b(cb);
				cb->setChecked(true);
			}
		}
		rebuild_log_components();
	});
	connect(sel_none, &QPushButton::clicked, this, [this]() {
		for (QCheckBox *cb : m_log_components) {
			if (cb->isEnabled()) {
				QSignalBlocker b(cb);
				cb->setChecked(false);
			}
		}
		rebuild_log_components();
	});
	sel_row->addWidget(sel_all);
	sel_row->addWidget(sel_none);
	sel_row->addStretch(1);
	comp_layout->addLayout(sel_row);
	config_form->addRow(comp_box);

	QLineEdit *file = new QLineEdit();
	file->setText(appcfg.log.file ? QString(appcfg.log.file) : QString());
	connect(file, &QLineEdit::editingFinished, this, [file]() {
		set_cstr((const char **) &appcfg.log.file, file->text());
	});
	QPushButton *file_browse = new QPushButton(tr("Browse..."));
	connect(file_browse, &QPushButton::clicked, this, [this, file]() {
		QString f = QFileDialog::getSaveFileName(this, tr("Log file"));
		if (!f.isNull()) file->setText(f);
	});
	QHBoxLayout *file_row = new QHBoxLayout();
	file_row->addWidget(file, 1);
	file_row->addWidget(file_browse);
	config_form->addRow(tr("Log file:"), file_row);

	QCheckBox *line_buffered = new QCheckBox(tr("Line buffered"));
	line_buffered->setChecked(appcfg.log.line_buffered);
	connect(line_buffered, &QCheckBox::toggled, this, [](bool on) {
		appcfg.log.line_buffered = on;
	});
	config_form->addRow(QString(), line_buffered);

	form->addRow(config_box);

	return page;
}

// -----------------------------------------------------------------------
void ConfigDialog::rebuild_log_components()
{
	QStringList names;
	for (QCheckBox *cb : m_log_components) {
		if (cb->isChecked()) {
			int id = cb->property("compid").toInt();
			names << QString(em400_log_component_name(id)).toLower();
		}
	}
	set_cstr((const char **) &appcfg.log.components, names.join(','));
}

// -----------------------------------------------------------------------
void ConfigDialog::reload_machine_page()
{
	bool have = machine != nullptr;

	const QObjectList edit_widgets = {
		m_name, m_awp, m_mod, m_user_io_illegal, m_nomem_stop, m_clock_period,
		m_elwro, m_mega, m_os_segments, m_mega_prom, m_preload
	};
	for (QObject *o : edit_widgets) {
		static_cast<QWidget *>(o)->setEnabled(have);
	}

	if (!have) return;

	const struct em400_machine_cfg *cfg = &machine->cfg;

	// blocked signals: loading values must not echo back as edits into the struct
	QSignalBlocker b_name(m_name);
	QSignalBlocker b_awp(m_awp);
	QSignalBlocker b_mod(m_mod);
	QSignalBlocker b_uioi(m_user_io_illegal);
	QSignalBlocker b_nomem(m_nomem_stop);
	QSignalBlocker b_clock(m_clock_period);
	QSignalBlocker b_elwro(m_elwro);
	QSignalBlocker b_mega(m_mega);
	QSignalBlocker b_osseg(m_os_segments);
	QSignalBlocker b_prom(m_mega_prom);
	QSignalBlocker b_preload(m_preload);

	m_name->setText(machine->name ? QString(machine->name) : QString());
	m_awp->setChecked(cfg->cpu.awp);
	m_mod->setChecked(cfg->cpu.mod);
	m_user_io_illegal->setChecked(cfg->cpu.user_io_illegal);
	m_nomem_stop->setChecked(cfg->cpu.nomem_stop);
	int clk_idx = m_clock_period->findData(cfg->cpu.clock_period_ms);
	m_clock_period->setCurrentIndex(clk_idx >= 0 ? clk_idx : 0);
	m_elwro->setValue(cfg->mem.elwro_modules);
	m_mega->setValue(cfg->mem.mega_modules);
	m_os_segments->setValue(cfg->mem.os_segments);
	m_mega_prom->setText(cfg->mem.mega_prom_image ? QString(cfg->mem.mega_prom_image) : QString());
	m_preload->setText(cfg->mem.preload_image ? QString(cfg->mem.preload_image) : QString());
}

// -----------------------------------------------------------------------
void ConfigDialog::slot_active_machine_changed(int index)
{
	(void) index;
	QString id = m_active->currentData().toString();
	set_cstr((const char **) &appcfg.active_id, id);
	machine = appcfg_machine_find(&appcfg, appcfg.active_id);
	reload_machine_page();
}

// vim: tabstop=4 shiftwidth=4 autoindent

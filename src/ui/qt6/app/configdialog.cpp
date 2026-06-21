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
#include <QSettings>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "configdialog.h"

// -----------------------------------------------------------------------
// strdup/free, not new/delete: appcfg_free() releases these with free().
namespace {
void set_cstr(const char **field, const QString &s)
{
	free((void *) *field);
	*field = s.isEmpty() ? nullptr : strdup(s.toUtf8().constData());
}

// free a device's owned union strings before its `type` is overwritten
void free_device_strings(struct em400_device_cfg *dev)
{
	switch (dev->type) {
	case EM400_DEV_WINCHESTER:
		free((void *) dev->winchester.image);
		break;
	case EM400_DEV_RTCLOCK:
		free((void *) dev->rtclock.prom);
		break;
	case EM400_DEV_SP45DE:
		for (int s=0 ; s<EM400_SP45DE_SLOT_COUNT ; s++) {
			free((void *) dev->sp45de.images[s]);
		}
		break;
	default:
		break;
	}
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
	sections->setMaximumWidth(130);
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

	setMinimumSize(720, 520);
}

// -----------------------------------------------------------------------
void ConfigDialog::add_section(const QString &title, const QString &icon_name, QWidget *page)
{
	QListWidgetItem *item = new QListWidgetItem(QIcon::fromTheme(icon_name), title, sections);
	item->setSizeHint(QSize(110, 38));
	stack->addWidget(page);
}

// -----------------------------------------------------------------------
QWidget *ConfigDialog::build_general_page()
{
	QWidget *page = new QWidget();
	QFormLayout *form = new QFormLayout(page);
	form->setVerticalSpacing(10);
	form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QCheckBox *powered = new QCheckBox(tr("Start with the machine powered on"));
	powered->setToolTip(tr("When off, the graphical UI starts with the machine powered down - turn the ignition key to power it on."));
	powered->setChecked(QSettings().value("ui/startPoweredOn", false).toBool());
	connect(powered, &QCheckBox::toggled, this, [](bool on) {
		QSettings().setValue("ui/startPoweredOn", on);
	});
	form->addRow(QString(), powered);

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
	QVBoxLayout *outer = new QVBoxLayout(page);

	QCheckBox *enabled = new QCheckBox(tr("Sound output enabled"));
	enabled->setChecked(appcfg.host.sound.enabled);
	outer->addWidget(enabled);

	QWidget *config_box = new QWidget();
	config_box->setEnabled(appcfg.host.sound.enabled);
	connect(enabled, &QCheckBox::toggled, this, [config_box](bool on) {
		appcfg.host.sound.enabled = on;
		config_box->setEnabled(on);
	});
	outer->addWidget(config_box);

	QFormLayout *form = new QFormLayout(config_box);
	form->setContentsMargins(0, 0, 0, 0);
	form->setVerticalSpacing(10);
	form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QSlider *volume = new QSlider(Qt::Horizontal);
	volume->setRange(0, 100);
	volume->setMinimumWidth(200);
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
	backend->setMinimumWidth(180);
	backend->setText(appcfg.host.sound.backend ? QString(appcfg.host.sound.backend) : QString());
	connect(backend, &QLineEdit::editingFinished, this, [backend]() {
		set_cstr(&appcfg.host.sound.backend, backend->text());
	});
	form->addRow(tr("Backend:"), backend);

	QLineEdit *device = new QLineEdit();
	device->setMinimumWidth(180);
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
	cpu->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
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
	mem->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
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
	m_mega_prom->setMinimumWidth(280);
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
	m_preload->setMinimumWidth(280);
	connect(m_preload, &QLineEdit::editingFinished, this, [this]() {
		if (machine) set_cstr(&machine->cfg.mem.preload_image, m_preload->text());
	});
	QPushButton *preload_browse = new QPushButton(tr("Browse..."));
	connect(preload_browse, &QPushButton::clicked, this, [this]() {
		QString f = QFileDialog::getOpenFileName(this, tr("Preload OS image"));
		if (!f.isNull()) m_preload->setText(f);
	});
	QHBoxLayout *preload_row = new QHBoxLayout();
	preload_row->addWidget(m_preload, 1);
	preload_row->addWidget(preload_browse);
	mem->addRow(tr("Preload OS image:"), preload_row);
	tabs->addTab(mem_box, tr("Memory"));

	tabs->addTab(build_io_page(), tr("I/O"));

	layout->addWidget(tabs);
	return page;
}

// -----------------------------------------------------------------------
QString ConfigDialog::chan_type_label(int type)
{
	switch (type) {
	case EM400_CHANNEL_CHAR:
		return tr("Character");
	case EM400_CHANNEL_MULTIX:
		return tr("MULTIX");
	case EM400_CHANNEL_IOTESTER:
		return tr("I/O tester");
	default:
		return tr("(empty)");
	}
}

// -----------------------------------------------------------------------
QString ConfigDialog::dev_type_label(int type)
{
	switch (type) {
	case EM400_DEV_TERMINAL:
		return tr("Terminal");
	case EM400_DEV_SP45DE:
		return tr("SP45DE (8\" floppy)");
	case EM400_DEV_WINCHESTER:
		return tr("Winchester");
	case EM400_DEV_FLOP5:
		return tr("Floppy (5.25\")");
	case EM400_DEV_RTCLOCK:
		return tr("Real-time clock");
	default:
		return tr("(none)");
	}
}

// -----------------------------------------------------------------------
QWidget *ConfigDialog::build_io_page()
{
	QWidget *page = new QWidget();
	QHBoxLayout *layout = new QHBoxLayout(page);

	io_tree = new QTreeWidget();
	io_tree->setHeaderHidden(true);
	io_tree->setMinimumWidth(140);
	io_tree->setMaximumWidth(220);
	connect(io_tree, &QTreeWidget::currentItemChanged, this, [this]() {
		io_selection_changed();
	});

	io_add_chan_btn = new QPushButton(tr("Add channel"));
	connect(io_add_chan_btn, &QPushButton::clicked, this, [this]() { io_add_channel(); });
	io_add_dev_btn = new QPushButton(tr("Add device"));
	connect(io_add_dev_btn, &QPushButton::clicked, this, [this]() { io_add_device(); });
	io_remove_btn = new QPushButton(tr("Remove"));
	connect(io_remove_btn, &QPushButton::clicked, this, [this]() { io_remove_selected(); });

	QHBoxLayout *add_row = new QHBoxLayout();
	add_row->setContentsMargins(0, 0, 0, 0);
	add_row->addWidget(io_add_chan_btn);
	add_row->addWidget(io_add_dev_btn);

	QVBoxLayout *btn_col = new QVBoxLayout();
	btn_col->setContentsMargins(0, 0, 0, 0);
	btn_col->addLayout(add_row);
	btn_col->addWidget(io_remove_btn);

	QVBoxLayout *left = new QVBoxLayout();
	left->setContentsMargins(0, 0, 0, 0);
	left->addWidget(new QLabel(tr("Channels and devices:")));
	left->addWidget(io_tree, 1);
	left->addLayout(btn_col);

	io_editor = new QStackedWidget();

	QLabel *placeholder = new QLabel(tr("Add a channel, then add devices to it."));
	placeholder->setAlignment(Qt::AlignCenter);
	placeholder->setWordWrap(true);
	io_editor->addWidget(placeholder);

	QWidget *chan_page = new QWidget();
	QFormLayout *chan_form = new QFormLayout(chan_page);
	chan_form->setVerticalSpacing(10);
	chan_form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
	io_chan_num = new QComboBox();
	connect(io_chan_num, &QComboBox::currentIndexChanged, this, [this]() {
		io_set_channel_number(io_chan_num->currentData().toInt());
	});
	chan_form->addRow(tr("Channel number:"), io_chan_num);
	io_chan_type = new QComboBox();
	io_chan_type->addItem(chan_type_label(EM400_CHANNEL_CHAR), EM400_CHANNEL_CHAR);
	io_chan_type->addItem(chan_type_label(EM400_CHANNEL_MULTIX), EM400_CHANNEL_MULTIX);
	io_chan_type->addItem(chan_type_label(EM400_CHANNEL_IOTESTER), EM400_CHANNEL_IOTESTER);
	connect(io_chan_type, &QComboBox::currentIndexChanged, this, [this]() {
		if (io_sel_chan < 0 || !machine) return;
		machine->cfg.channel[io_sel_chan].type =
			(enum em400_channel_types) io_chan_type->currentData().toInt();
		io_build_tree();
	});
	chan_form->addRow(tr("Channel type:"), io_chan_type);
	io_editor->addWidget(chan_page);

	QWidget *dev_page = new QWidget();
	QVBoxLayout *dev_layout = new QVBoxLayout(dev_page);
	QFormLayout *dev_form = new QFormLayout();
	dev_form->setContentsMargins(0, 0, 0, 0);
	dev_form->setVerticalSpacing(10);
	dev_form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
	io_dev_num = new QComboBox();
	connect(io_dev_num, &QComboBox::currentIndexChanged, this, [this]() {
		io_set_device_number(io_dev_num->currentData().toInt());
	});
	dev_form->addRow(tr("Device number:"), io_dev_num);
	io_dev_type = new QComboBox();
	io_dev_type->addItem(dev_type_label(EM400_DEV_TERMINAL), EM400_DEV_TERMINAL);
	io_dev_type->addItem(dev_type_label(EM400_DEV_WINCHESTER), EM400_DEV_WINCHESTER);
	io_dev_type->addItem(dev_type_label(EM400_DEV_SP45DE), EM400_DEV_SP45DE);
	io_dev_type->addItem(dev_type_label(EM400_DEV_FLOP5), EM400_DEV_FLOP5);
	io_dev_type->addItem(dev_type_label(EM400_DEV_RTCLOCK), EM400_DEV_RTCLOCK);
	connect(io_dev_type, &QComboBox::currentIndexChanged, this, [this]() {
		if (io_sel_chan < 0 || io_sel_dev < 0 || !machine) return;
		struct em400_device_cfg *dev = &machine->cfg.channel[io_sel_chan].device[io_sel_dev];
		enum em400_device_types want = (enum em400_device_types) io_dev_type->currentData().toInt();
		if (dev->type == want) return;
		free_device_strings(dev);
		*dev = (struct em400_device_cfg){};
		dev->type = want;
		io_rebuild_dev_params();
		io_build_tree();
	});
	dev_form->addRow(tr("Device type:"), io_dev_type);
	dev_layout->addLayout(dev_form);

	io_dev_params = new QWidget();
	new QFormLayout(io_dev_params);
	dev_layout->addWidget(io_dev_params);
	dev_layout->addStretch(1);
	io_editor->addWidget(dev_page);

	layout->addLayout(left);
	layout->addWidget(io_editor, 1);

	io_build_tree();
	io_update_buttons();
	return page;
}

// -----------------------------------------------------------------------
// Tree lists only configured channels/devices; empty slots are reached via
// the Add buttons, not shown as noise.
void ConfigDialog::io_build_tree()
{
	QSignalBlocker b(io_tree);
	io_tree->clear();
	if (!machine) return;

	QTreeWidgetItem *restore = nullptr;

	for (int ch=0 ; ch<EM400_IO_MAX_CHAN ; ch++) {
		struct em400_channel_cfg *chan = &machine->cfg.channel[ch];
		if (chan->type == EM400_CHANNEL_NONE) continue;

		QTreeWidgetItem *ci = new QTreeWidgetItem(io_tree);
		ci->setText(0, tr("%1: %2").arg(ch).arg(chan_type_label(chan->type)));
		ci->setData(0, Qt::UserRole, ch);
		ci->setData(0, Qt::UserRole + 1, -1);
		if (ch == io_sel_chan && io_sel_dev < 0) {
			restore = ci;
		}

		for (int d=0 ; d<EM400_CHAN_MAX_DEV ; d++) {
			struct em400_device_cfg *dev = &chan->device[d];
			if (dev->type == EM400_DEV_NONE) continue;

			QTreeWidgetItem *di = new QTreeWidgetItem(ci);
			di->setText(0, tr("%1: %2").arg(d).arg(dev_type_label(dev->type)));
			di->setData(0, Qt::UserRole, ch);
			di->setData(0, Qt::UserRole + 1, d);
			if (ch == io_sel_chan && d == io_sel_dev) {
				restore = di;
			}
		}
		ci->setExpanded(true);
	}

	if (restore) {
		io_tree->setCurrentItem(restore);
	}
}

// -----------------------------------------------------------------------
void ConfigDialog::io_selection_changed()
{
	QTreeWidgetItem *item = io_tree->currentItem();
	if (!item) {
		io_sel_chan = io_sel_dev = -1;
		io_editor->setCurrentIndex(0);
		io_update_buttons();
		return;
	}

	io_sel_chan = item->data(0, Qt::UserRole).toInt();
	io_sel_dev = item->data(0, Qt::UserRole + 1).toInt();

	if (io_sel_dev < 0) {
		QSignalBlocker bn(io_chan_num);
		io_chan_num->clear();
		for (int i=0 ; i<EM400_IO_MAX_CHAN ; i++) {
			if (i == io_sel_chan || machine->cfg.channel[i].type == EM400_CHANNEL_NONE) {
				io_chan_num->addItem(QString::number(i), i);
			}
		}
		io_chan_num->setCurrentIndex(io_chan_num->findData(io_sel_chan));

		QSignalBlocker bt(io_chan_type);
		int idx = io_chan_type->findData(machine->cfg.channel[io_sel_chan].type);
		io_chan_type->setCurrentIndex(idx >= 0 ? idx : 0);
		io_editor->setCurrentIndex(1);
	} else {
		const struct em400_channel_cfg *chan = &machine->cfg.channel[io_sel_chan];

		QSignalBlocker bn(io_dev_num);
		io_dev_num->clear();
		for (int i=0 ; i<EM400_CHAN_MAX_DEV ; i++) {
			if (i == io_sel_dev || chan->device[i].type == EM400_DEV_NONE) {
				io_dev_num->addItem(QString::number(i), i);
			}
		}
		io_dev_num->setCurrentIndex(io_dev_num->findData(io_sel_dev));

		QSignalBlocker bt(io_dev_type);
		int idx = io_dev_type->findData(chan->device[io_sel_dev].type);
		io_dev_type->setCurrentIndex(idx >= 0 ? idx : 0);
		io_rebuild_dev_params();
		io_editor->setCurrentIndex(2);
	}

	io_update_buttons();
}

// -----------------------------------------------------------------------
void ConfigDialog::io_update_buttons()
{
	bool have_machine = machine != nullptr;
	bool chan_full = true;
	if (have_machine) {
		for (int i=0 ; i<EM400_IO_MAX_CHAN ; i++) {
			if (machine->cfg.channel[i].type == EM400_CHANNEL_NONE) {
				chan_full = false;
				break;
			}
		}
	}
	io_add_chan_btn->setEnabled(have_machine && !chan_full);
	io_add_dev_btn->setEnabled(io_sel_chan >= 0);
	io_remove_btn->setEnabled(io_sel_chan >= 0);
}

// -----------------------------------------------------------------------
void ConfigDialog::io_add_channel()
{
	if (!machine) return;
	for (int i=0 ; i<EM400_IO_MAX_CHAN ; i++) {
		if (machine->cfg.channel[i].type == EM400_CHANNEL_NONE) {
			machine->cfg.channel[i].type = EM400_CHANNEL_CHAR;
			io_sel_chan = i;
			io_sel_dev = -1;
			io_build_tree();
			io_selection_changed();
			return;
		}
	}
}

// -----------------------------------------------------------------------
void ConfigDialog::io_add_device()
{
	if (!machine || io_sel_chan < 0) return;
	struct em400_channel_cfg *chan = &machine->cfg.channel[io_sel_chan];
	for (int d=0 ; d<EM400_CHAN_MAX_DEV ; d++) {
		if (chan->device[d].type == EM400_DEV_NONE) {
			chan->device[d].type = EM400_DEV_TERMINAL;
			chan->device[d].terminal.speed = 9600;
			io_sel_dev = d;
			io_build_tree();
			io_selection_changed();
			return;
		}
	}
}

// -----------------------------------------------------------------------
void ConfigDialog::io_remove_selected()
{
	if (!machine || io_sel_chan < 0) return;
	struct em400_channel_cfg *chan = &machine->cfg.channel[io_sel_chan];

	if (io_sel_dev >= 0) {
		struct em400_device_cfg *dev = &chan->device[io_sel_dev];
		free_device_strings(dev);
		*dev = (struct em400_device_cfg){};
		io_sel_dev = -1;
	} else {
		for (int d=0 ; d<EM400_CHAN_MAX_DEV ; d++) {
			free_device_strings(&chan->device[d]);
		}
		*chan = (struct em400_channel_cfg){};
		io_sel_chan = -1;
	}

	io_build_tree();
	io_selection_changed();
}

// -----------------------------------------------------------------------
// The target number is guaranteed free (the combo offers only free slots +
// the current one), so ownership of any union strings just moves with the copy.
void ConfigDialog::io_set_channel_number(int num)
{
	if (!machine || io_sel_chan < 0 || num == io_sel_chan) return;
	machine->cfg.channel[num] = machine->cfg.channel[io_sel_chan];
	machine->cfg.channel[io_sel_chan] = (struct em400_channel_cfg){};
	io_sel_chan = num;
	io_build_tree();
	io_selection_changed();
}

// -----------------------------------------------------------------------
void ConfigDialog::io_set_device_number(int num)
{
	if (!machine || io_sel_chan < 0 || io_sel_dev < 0 || num == io_sel_dev) return;
	struct em400_channel_cfg *chan = &machine->cfg.channel[io_sel_chan];
	chan->device[num] = chan->device[io_sel_dev];
	chan->device[io_sel_dev] = (struct em400_device_cfg){};
	io_sel_dev = num;
	io_build_tree();
	io_selection_changed();
}

// -----------------------------------------------------------------------
void ConfigDialog::io_rebuild_dev_params()
{
	delete io_dev_params->layout();
	qDeleteAll(io_dev_params->findChildren<QWidget *>("", Qt::FindDirectChildrenOnly));

	QFormLayout *form = new QFormLayout(io_dev_params);
	form->setContentsMargins(0, 0, 0, 0);
	form->setVerticalSpacing(10);

	if (io_sel_chan < 0 || io_sel_dev < 0 || !machine) return;

	const int ch = io_sel_chan, d = io_sel_dev;
	struct em400_device_cfg *dev = &machine->cfg.channel[ch].device[d];

	auto image_row = [this, form](const QString &label, const char **field, const QString &caption) {
		QLineEdit *edit = new QLineEdit();
		edit->setText(*field ? QString(*field) : QString());
		connect(edit, &QLineEdit::editingFinished, this, [field, edit]() {
			set_cstr(field, edit->text());
		});
		QPushButton *browse = new QPushButton(tr("Browse..."));
		connect(browse, &QPushButton::clicked, this, [this, edit, caption]() {
			QString f = QFileDialog::getOpenFileName(this, caption);
			if (!f.isNull()) edit->setText(f);
		});
		QHBoxLayout *row = new QHBoxLayout();
		row->addWidget(edit, 1);
		row->addWidget(browse);
		form->addRow(label, row);
	};

	switch (dev->type) {
	case EM400_DEV_TERMINAL: {
		QSpinBox *port = new QSpinBox();
		port->setRange(0, 65535);
		port->setMaximumWidth(140);
		port->setValue(dev->terminal.port);
		connect(port, &QSpinBox::valueChanged, this, [this, ch, d](int v) {
			machine->cfg.channel[ch].device[d].terminal.port = v;
		});
		form->addRow(tr("TCP port:"), port);

		QComboBox *speed = new QComboBox();
		speed->setMaximumWidth(140);
		for (int baud : {150, 300, 600, 1200, 2400, 4800, 9600}) {
			speed->addItem(QString::number(baud), baud);
		}
		int sidx = speed->findData(dev->terminal.speed ? dev->terminal.speed : 9600);
		speed->setCurrentIndex(sidx >= 0 ? sidx : speed->findData(9600));
		connect(speed, &QComboBox::currentIndexChanged, this, [this, ch, d, speed]() {
			machine->cfg.channel[ch].device[d].terminal.speed = speed->currentData().toInt();
		});
		form->addRow(tr("Speed (baud):"), speed);
		break;
	}
	case EM400_DEV_WINCHESTER:
		image_row(tr("Disk image:"), &dev->winchester.image, tr("Winchester disk image"));
		break;
	case EM400_DEV_RTCLOCK:
		image_row(tr("PROM image:"), &dev->rtclock.prom, tr("RTC PROM image"));
		break;
	case EM400_DEV_SP45DE:
		for (int s=0 ; s<EM400_SP45DE_SLOT_COUNT ; s++) {
			image_row(tr("Slot %1 image:").arg(s), &dev->sp45de.images[s], tr("Floppy image"));
		}
		break;
	default:
		break;
	}
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

	io_sel_chan = io_sel_dev = -1;
	if (io_tree) {
		io_build_tree();
		io_editor->setCurrentIndex(0);
		io_update_buttons();
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

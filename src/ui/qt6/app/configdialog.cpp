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

#include <QApplication>
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
#include <QMessageBox>
#include <QFontDialog>
#include <QFontInfo>
#include <QTextDocument>
#include <QtMath>

#include "configdialog.h"
#include "configcontroller.h"
#include "theme.h"

// -----------------------------------------------------------------------
// strdup/free, not new/delete: appcfg_free() releases these with free().
namespace {

// QLabel's rich-text height hint doesn't match what it renders and reserves
// phantom vertical space; take the height from the QTextDocument instead.
class RichLabel : public QLabel {
public:
	using QLabel::QLabel;
	bool hasHeightForWidth() const override { return true; }
	int heightForWidth(int w) const override
	{
		QTextDocument doc;
		doc.setDefaultFont(font());
		if (Qt::mightBeRichText(text())) doc.setHtml(text()); else doc.setPlainText(text());
		doc.setTextWidth(w);
		return qCeil(doc.size().height());
	}
	QSize sizeHint() const override { return QSize(0, heightForWidth(width() > 0 ? width() : 400)); }
	QSize minimumSizeHint() const override { return sizeHint(); }
};

void set_cstr(const char **field, const QString &s)
{
	free((void *) *field);
	*field = s.isEmpty() ? nullptr : strdup(s.toUtf8().constData());
}

// Tag a control with its editability lifecycle, read back by
// update_enabled_states(): "live" = always editable, "cold" = only while
// powered off, "media:c:d:s" = off or em400_dev_can_eject(c,d,s). Untagged
// widgets (labels, containers, navigation) are left alone.
void gate(QWidget *w, const QString &kind)
{
	w->setProperty("gate", kind);
}

// [machine.<id>] must survive the INI round-trip: ASCII lowercase letters and
// digits only. NFD strips most diacritics, but the Polish stroke (l/L) doesn't decompose.
QString derive_machine_id(const QString &name)
{
	QString id;
	for (QChar c : name.normalized(QString::NormalizationForm_D)) {
		if ((c == QChar(u'ł')) || (c == QChar(u'Ł'))) {
			id += QLatin1Char('l');
		} else if ((c.unicode() < 128) && c.isLetterOrNumber()) {
			id += c.toLower();
		}
	}
	return id.isEmpty() ? QStringLiteral("machine") : id;
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
ConfigDialog::ConfigDialog(ConfigController *ctl, QWidget *parent) :
	QDialog(parent),
	ctl(ctl)
{
	setWindowTitle(tr("Settings"));

	// edit a private deep copy; OK commits it back to appcfg, Cancel discards it
	appcfg_copy(&work, &appcfg);
	orig_volume = work.host.sound.volume;

	{
		QSettings s;
		orig_mono_font_set = s.contains("ui/monoFontFamily");
		orig_mono_font_family = s.value("ui/monoFontFamily").toString();
		orig_mono_font_size = s.value("ui/monoFontSize", 0).toInt();
	}

	machine = appcfg_machine_find(&work, work.active_id);
	if (!machine && work.n_machines) {
		machine = &work.machines[0];
	}

	sections = new QListWidget();
	sections->setMaximumWidth(130);
	sections->setIconSize(QSize(24, 24));
	stack = new QStackedWidget();

	machine_page = build_machine_page();
	add_section(tr("Machine"), "computer", machine_page);
	add_section(tr("General"), "preferences-system", build_general_page());
	add_section(tr("Sound"), "audio-volume-high", build_sound_page());
	add_section(tr("Logging"), "text-x-generic", build_log_page());

	reload_machine_page();

	connect(sections, &QListWidget::currentRowChanged, stack, &QStackedWidget::setCurrentIndex);
	sections->setCurrentRow(0);

	QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttons, &QDialogButtonBox::accepted, this, &ConfigDialog::accept_config);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

	QHBoxLayout *body = new QHBoxLayout();
	body->addWidget(sections);
	body->addWidget(stack, 1);

	QVBoxLayout *outer = new QVBoxLayout(this);
	outer->addLayout(body);
	outer->addWidget(buttons);

	setMinimumSize(720, 520);

	// pages are children of `this` only now that the layout is built, so the
	// reload_machine_page() pass above could not reach them via findChildren
	update_enabled_states();
}

// -----------------------------------------------------------------------
ConfigDialog::~ConfigDialog()
{
	appcfg_free_contents(&work);
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
	QVBoxLayout *outer = new QVBoxLayout(page);

	QGroupBox *emu_box = new QGroupBox(tr("Emulation"));
	QFormLayout *emu_form = new QFormLayout(emu_box);
	emu_form->setVerticalSpacing(10);
	emu_form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

	QComboBox *timing = new QComboBox();
	timing->addItem(tr("All"), EM400_TIMING_ALL);
	timing->addItem(tr("Minimal"), EM400_TIMING_MINIMAL);
	timing->addItem(tr("None"), EM400_TIMING_NONE);
	timing->setCurrentIndex(timing->findData(work.host.emu.timing));
	gate(timing, "cold");
	emu_form->addRow(tr("Emulated timings:"), timing);

	RichLabel *desc_text = new RichLabel();
	desc_text->setWordWrap(true);
	desc_text->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	// without Expanding the field column stays at the combo's narrow sizeHint and the text wraps early
	QSizePolicy desc_sp = desc_text->sizePolicy();
	desc_sp.setHorizontalPolicy(QSizePolicy::Expanding);
	desc_text->setSizePolicy(desc_sp);
	emu_form->addRow(QString(), desc_text);

	auto set_timing_desc = [desc_text](enum em400_timing t) {
		switch (t) {
		case EM400_TIMING_NONE: {
			QString msg = tr("WARNING! For testing and debugging purposes. May break the real software.");
			int colon = msg.indexOf(QLatin1Char('!'));
			QString lead = msg.left(colon + 1).toHtmlEscaped();
			QString rest = msg.mid(colon + 1).toHtmlEscaped();
			desc_text->setText(QStringLiteral("<span style=\"color:red\"><b>%1</b></span>%2").arg(lead, rest));
			break;
		}
		case EM400_TIMING_MINIMAL:
			desc_text->setText(tr("Only the timings required for MERA-400 software to run correctly are emulated."));
			break;
		default:
			desc_text->setText(tr("All timings the emulator supports are active. EM400 runs as close to the real MERA-400 as possible."));
			break;
		}
	};
	set_timing_desc(work.host.emu.timing);

	connect(timing, &QComboBox::currentIndexChanged, this, [this, timing, set_timing_desc]() {
		enum em400_timing t = (enum em400_timing) timing->currentData().toInt();
		work.host.emu.timing = t;
		set_timing_desc(t);
	});

	QSpinBox *quantum = new QSpinBox();
	quantum->setRange(50, 900);
	quantum->setSingleStep(10);
	quantum->setSuffix(tr(" us"));
	quantum->setValue(work.host.emu.emulation_quantum_us);
	connect(quantum, &QSpinBox::valueChanged, this, [this](int v) {
		work.host.emu.emulation_quantum_us = v;
	});
	gate(quantum, "cold");
	emu_form->addRow(tr("Emulation quantum:"), quantum);

	QGroupBox *ui_box = new QGroupBox(tr("User interface"));
	QFormLayout *ui_form = new QFormLayout(ui_box);
	ui_form->setVerticalSpacing(10);
	ui_form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

	// language names stay untranslated on purpose: each must be readable to a
	// user stuck in the other language
	QComboBox *language = new QComboBox();
	language->addItem(tr("System default"), QString());
	language->addItem(QStringLiteral("English"), QStringLiteral("en"));
	language->addItem(QStringLiteral("Polski"), QStringLiteral("pl"));
	int lang_idx = language->findData(QSettings().value("ui/language").toString());
	language->setCurrentIndex(lang_idx >= 0 ? lang_idx : 0);
	QLabel *language_note = new QLabel(tr("Takes effect after EM400 is restarted."));
	auto lang_pending = [language]() {
		return language->currentData().toString() != qApp->property("startupLanguage").toString();
	};
	language_note->setVisible(lang_pending());
	connect(language, &QComboBox::currentIndexChanged, this, [language, language_note, lang_pending]() {
		QSettings().setValue("ui/language", language->currentData().toString());
		language_note->setVisible(lang_pending());
	});
	gate(language, "live");
	QHBoxLayout *language_row = new QHBoxLayout();
	language_row->addWidget(language);
	language_row->addSpacing(12);
	language_row->addWidget(language_note);
	language_row->addStretch(1);
	ui_form->addRow(tr("Language:"), language_row);

	QCheckBox *powered = new QCheckBox(tr("Start with the machine powered on"));
	powered->setToolTip(tr("When off, the graphical UI starts with the machine powered down.\nTurn the ignition key to power it on."));
	powered->setChecked(QSettings().value("ui/startPoweredOn", false).toBool());
	connect(powered, &QCheckBox::toggled, this, [](bool on) {
		QSettings().setValue("ui/startPoweredOn", on);
	});
	gate(powered, "live");
	ui_form->addRow(QString(), powered);

	QCheckBox *panel_theme = new QCheckBox(tr("Panel theme"));
	panel_theme->setToolTip(tr("Style the whole UI to match the MERA-400 control panel.\nWhen off, the system theme is used."));
	panel_theme->setChecked(QSettings().value("ui/panelTheme", true).toBool());
	connect(panel_theme, &QCheckBox::toggled, this, [this](bool on) {
		emit signal_panel_theme_changed(on);
	});
	gate(panel_theme, "live");
	ui_form->addRow(QString(), panel_theme);

	QCheckBox *small_cp = new QCheckBox(tr("Small control panel"));
	small_cp->setChecked(QSettings().value("layout/smallPanel", false).toBool());
	connect(small_cp, &QCheckBox::toggled, this, [this](bool on) {
		emit signal_small_cp_changed(on);
	});
	gate(small_cp, "live");
	ui_form->addRow(QString(), small_cp);

	QComboBox *terminal_mode = new QComboBox();
	terminal_mode->setToolTip(tr("How Devices -> Open terminal connects to a terminal device.\nThe built-in terminal needs no external program.\nThe external one runs a program of your choice."));
	terminal_mode->addItem(tr("Built-in"), "builtin");
	terminal_mode->addItem(tr("External"), "external");
	QString cur_mode = QSettings().value("ui/terminalMode", "builtin").toString();
	terminal_mode->setCurrentIndex(qMax(0, terminal_mode->findData(cur_mode)));

	QLineEdit *terminal_cmd = new QLineEdit();
	terminal_cmd->setToolTip(tr("Command launched by Devices -> Open terminal when the external mode is selected.\n{port} is replaced with the terminal device's TCP port.\nThe default uses the bundled emterm helper."));
	terminal_cmd->setText(QSettings().value("ui/terminalCommand", "xterm -e emterm {port}").toString());
	terminal_cmd->setEnabled(cur_mode == "external");
	connect(terminal_cmd, &QLineEdit::editingFinished, this, [terminal_cmd]() {
		QSettings().setValue("ui/terminalCommand", terminal_cmd->text());
	});
	connect(terminal_mode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [terminal_mode, terminal_cmd](int) {
		QString mode = terminal_mode->currentData().toString();
		QSettings().setValue("ui/terminalMode", mode);
		terminal_cmd->setEnabled(mode == "external");
	});
	ui_form->addRow(tr("Terminal:"), terminal_mode);
	ui_form->addRow(tr("Terminal command:"), terminal_cmd);

	// Debugger monospace font (memory/disassembly/registers/...)
	// Empty = the platform default resolved by em400_apply_mono_font()
	QLabel *font_label = new QLabel();
	font_label->setToolTip(tr("Font used by the memory, disassembly, register and other debugger views."));
	auto describe_font = [this, font_label]() {
		QSettings s;
		const QString fam = s.value("ui/monoFontFamily").toString();
		const int size = s.value("ui/monoFontSize", 0).toInt();
		QFont f;
		em400_apply_mono_font(f);
		const QString name = fam.isEmpty() ? QFontInfo(f).family() : fam;
		const int pt = size > 0 ? size : QFontInfo(f).pointSize();
		font_label->setText(fam.isEmpty() && size <= 0
			? tr("%1 %2 pt (default)").arg(name).arg(pt)
			: tr("%1 %2 pt").arg(name).arg(pt));
	};
	describe_font();
	QPushButton *font_change = new QPushButton(tr("Change..."));
	connect(font_change, &QPushButton::clicked, this, [this, describe_font]() {
		QFont initial;
		em400_apply_mono_font(initial);
		bool ok = false;
		QFont chosen = QFontDialog::getFont(&ok, initial, this, tr("Monospace font"), QFontDialog::MonospacedFonts);
		if (!ok) return;
		QSettings s;
		s.setValue("ui/monoFontFamily", chosen.family());
		s.setValue("ui/monoFontSize", chosen.pointSize());
		mono_font_touched = true;
		describe_font();
		emit signal_mono_font_changed();
	});
	QPushButton *font_reset = new QPushButton(tr("Reset"));
	connect(font_reset, &QPushButton::clicked, this, [this, describe_font]() {
		QSettings s;
		s.remove("ui/monoFontFamily");
		s.remove("ui/monoFontSize");
		mono_font_touched = true;
		describe_font();
		emit signal_mono_font_changed();
	});
	QHBoxLayout *font_row = new QHBoxLayout();
	font_row->addWidget(font_label, 1);
	font_row->addWidget(font_change);
	font_row->addWidget(font_reset);
	ui_form->addRow(tr("Debugger font:"), font_row);

	QLabel *term_font_label = new QLabel();
	term_font_label->setToolTip(tr("Font used by the built-in terminal."));
	auto describe_term_font = [this, term_font_label]() {
		QSettings s;
		const QString fam = s.value("ui/terminalFontFamily").toString();
		const int size = s.value("ui/terminalFontSize", 0).toInt();
		QFont f;
		em400_apply_terminal_font(f);
		const QString name = fam.isEmpty() ? QFontInfo(f).family() : fam;
		const int pt = size > 0 ? size : QFontInfo(f).pointSize();
		term_font_label->setText(fam.isEmpty() && size <= 0
			? tr("%1 %2 pt (default)").arg(name).arg(pt)
			: tr("%1 %2 pt").arg(name).arg(pt));
	};
	describe_term_font();
	QPushButton *term_font_change = new QPushButton(tr("Change..."));
	connect(term_font_change, &QPushButton::clicked, this, [this, describe_term_font]() {
		QFont initial;
		em400_apply_terminal_font(initial);
		bool ok = false;
		QFont chosen = QFontDialog::getFont(&ok, initial, this, tr("Terminal font"), QFontDialog::MonospacedFonts);
		if (!ok) return;
		QSettings s;
		s.setValue("ui/terminalFontFamily", chosen.family());
		s.setValue("ui/terminalFontSize", chosen.pointSize());
		describe_term_font();
		emit signal_terminal_font_changed();
	});
	QPushButton *term_font_reset = new QPushButton(tr("Reset"));
	connect(term_font_reset, &QPushButton::clicked, this, [this, describe_term_font]() {
		QSettings s;
		s.remove("ui/terminalFontFamily");
		s.remove("ui/terminalFontSize");
		describe_term_font();
		emit signal_terminal_font_changed();
	});
	QHBoxLayout *term_font_row = new QHBoxLayout();
	term_font_row->addWidget(term_font_label, 1);
	term_font_row->addWidget(term_font_change);
	term_font_row->addWidget(term_font_reset);
	ui_form->addRow(tr("Terminal font:"), term_font_row);

	outer->addWidget(emu_box);
	outer->addWidget(ui_box);
	outer->addStretch();

	return page;
}

// -----------------------------------------------------------------------
QWidget *ConfigDialog::build_sound_page()
{
	QWidget *page = new QWidget();
	QVBoxLayout *outer = new QVBoxLayout(page);

	QGroupBox *gui_box = new QGroupBox(tr("GUI sounds"));
	QFormLayout *gui_form = new QFormLayout(gui_box);
	gui_form->setVerticalSpacing(10);
	gui_form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QSlider *gui_volume = new QSlider(Qt::Horizontal);
	gui_volume->setRange(0, 100);
	gui_volume->setMinimumWidth(200);
	gui_volume->setValue(QSettings().value("ui/guiVolume", 100).toInt());
	QLabel *gui_volume_val = new QLabel(QString::number(gui_volume->value()));
	gui_volume_val->setMinimumWidth(gui_volume_val->fontMetrics().horizontalAdvance("100"));
	connect(gui_volume, &QSlider::valueChanged, this, [this, gui_volume_val](int v) {
		QSettings().setValue("ui/guiVolume", v);
		gui_volume_val->setText(QString::number(v));
		emit signal_gui_volume_changed(v);
	});
	gate(gui_volume, "live");
	QHBoxLayout *gui_volume_row = new QHBoxLayout();
	gui_volume_row->addWidget(gui_volume, 1);
	gui_volume_row->addWidget(gui_volume_val);
	gui_form->addRow(tr("Volume:"), gui_volume_row);

	QCheckBox *coil_whine = new QCheckBox(tr("I love when coils whine"));
	coil_whine->setChecked(QSettings().value("ui/psuSound", false).toBool());
	connect(coil_whine, &QCheckBox::toggled, this, [this](bool on) {
		QSettings().setValue("ui/psuSound", on);
		emit signal_psu_sound_changed(on);
	});
	gate(coil_whine, "live");
	gui_form->addRow(QString(), coil_whine);
	outer->addWidget(gui_box);

	QGroupBox *buzzer_box = new QGroupBox(tr("CPU speaker"));
	QVBoxLayout *buzzer_layout = new QVBoxLayout(buzzer_box);

	QCheckBox *enabled = new QCheckBox(tr("Sound output enabled"));
	enabled->setChecked(work.host.sound.enabled);
	gate(enabled, "cold");
	buzzer_layout->addWidget(enabled);

	QWidget *config_box = new QWidget();
	connect(enabled, &QCheckBox::toggled, this, [this](bool on) {
		work.host.sound.enabled = on;
	});
	buzzer_layout->addWidget(config_box);

	QFormLayout *form = new QFormLayout(config_box);
	form->setContentsMargins(0, 0, 0, 0);
	form->setVerticalSpacing(10);
	form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QSlider *volume = new QSlider(Qt::Horizontal);
	volume->setRange(0, 100);
	volume->setMinimumWidth(200);
	volume->setValue(work.host.sound.volume);
	QLabel *volume_val = new QLabel(QString::number(work.host.sound.volume));
	volume_val->setMinimumWidth(volume_val->fontMetrics().horizontalAdvance("100"));
	connect(volume, &QSlider::valueChanged, this, [this, volume_val](int v) {
		work.host.sound.volume = v;
		ctl->preview_volume(v);
		volume_val->setText(QString::number(v));
	});
	gate(volume, "live");
	QHBoxLayout *volume_row = new QHBoxLayout();
	volume_row->addWidget(volume, 1);
	volume_row->addWidget(volume_val);
	form->addRow(tr("Volume:"), volume_row);

	QComboBox *rate = new QComboBox();
	for (int r : {8000, 11025, 16000, 22050, 32000, 44100, 48000, 88200, 96000, 192000}) {
		rate->addItem(tr("%1 Hz").arg(r), r);
	}
	if (rate->findData(work.host.sound.sample_rate) < 0) {
		rate->addItem(tr("%1 Hz").arg(work.host.sound.sample_rate), work.host.sound.sample_rate);
	}
	rate->setCurrentIndex(rate->findData(work.host.sound.sample_rate));
	connect(rate, &QComboBox::currentIndexChanged, this, [this, rate]() {
		work.host.sound.sample_rate = rate->currentData().toInt();
	});
	gate(rate, "cold");
	form->addRow(tr("Sample rate:"), rate);

	QComboBox *buffer = new QComboBox();
	for (int b=16 ; b<=8192 ; b*=2) {
		buffer->addItem(QString::number(b), b);
	}
	if (buffer->findData(work.host.sound.buffer_len) < 0) {
		buffer->addItem(QString::number(work.host.sound.buffer_len), work.host.sound.buffer_len);
	}
	buffer->setCurrentIndex(buffer->findData(work.host.sound.buffer_len));
	connect(buffer, &QComboBox::currentIndexChanged, this, [this, buffer]() {
		work.host.sound.buffer_len = buffer->currentData().toInt();
	});
	gate(buffer, "cold");
	form->addRow(tr("Buffer length (frames):"), buffer);

	QSpinBox *latency = new QSpinBox();
	latency->setRange(0, 1000);
	latency->setSuffix(tr(" ms"));
	latency->setValue(work.host.sound.latency);
	connect(latency, &QSpinBox::valueChanged, this, [this](int v) {
		work.host.sound.latency = v;
	});
	gate(latency, "cold");
	form->addRow(tr("Latency:"), latency);

	QLineEdit *backend = new QLineEdit();
	backend->setMinimumWidth(180);
	backend->setText(work.host.sound.backend ? QString(work.host.sound.backend) : QString());
	connect(backend, &QLineEdit::editingFinished, this, [this, backend]() {
		set_cstr(&work.host.sound.backend, backend->text());
	});
	gate(backend, "cold");
	form->addRow(tr("Audio backend:"), backend);

	QLineEdit *device = new QLineEdit();
	device->setMinimumWidth(180);
	device->setText(work.host.sound.device ? QString(work.host.sound.device) : QString());
	connect(device, &QLineEdit::editingFinished, this, [this, device]() {
		set_cstr(&work.host.sound.device, device->text());
	});
	gate(device, "cold");
	form->addRow(tr("Device:"), device);

	outer->addWidget(buzzer_box);
	outer->addStretch();

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
	for (int i=0 ; i<work.n_machines ; i++) {
		const struct appcfg_machine *m = &work.machines[i];
		m_active->addItem(m->name ? QString(m->name) : QString(m->id), QString(m->id));
	}
	int act_idx = m_active->findData(work.active_id ? QString(work.active_id) : QString());
	if (act_idx >= 0) m_active->setCurrentIndex(act_idx);
	connect(m_active, &QComboBox::currentIndexChanged, this, &ConfigDialog::slot_active_machine_changed);
	gate(m_active, "cold");

	// not gate()d: Add must stay usable when no machine exists, while the generic
	// pass disables all gated machine-page widgets then; see update_enabled_states()
	m_add_machine = new QPushButton(tr("Add..."));
	connect(m_add_machine, &QPushButton::clicked, this, [this]() { machine_add(false); });
	m_dup_machine = new QPushButton(tr("Duplicate..."));
	connect(m_dup_machine, &QPushButton::clicked, this, [this]() { machine_add(true); });
	m_del_machine = new QPushButton(tr("Delete"));
	connect(m_del_machine, &QPushButton::clicked, this, [this]() { machine_delete(); });

	QHBoxLayout *active_row = new QHBoxLayout();
	active_row->addWidget(m_active, 1);
	active_row->addWidget(m_add_machine);
	active_row->addWidget(m_dup_machine);
	active_row->addWidget(m_del_machine);
	id_form->addRow(tr("Active machine:"), active_row);

	m_name = new QLineEdit();
	connect(m_name, &QLineEdit::editingFinished, this, [this]() {
		if (!machine) return;
		appcfg_machine_set_name(machine, m_name->text().toUtf8().constData());
		m_active->setItemText(m_active->currentIndex(),
			machine->name ? QString(machine->name) : QString(machine->id));
	});
	gate(m_name, "live");
	id_form->addRow(tr("Name:"), m_name);

	m_id = new QLabel();
	m_id->setEnabled(false);
	m_id->setToolTip(tr("Fixed at machine creation.\nIdentifies the machine in the configuration file\nand for the -m command line option."));
	id_form->addRow(tr("Identifier:"), m_id);
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
	m_user_io_illegal = new QCheckBox(tr("I/O instructions illegal in user mode"));
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
	for (QWidget *w : std::initializer_list<QWidget *>{m_awp, m_mod, m_user_io_illegal, m_nomem_stop, m_clock_period}) {
		gate(w, "cold");
	}
	tabs->addTab(cpu_box, tr("CPU"));

	QWidget *mem_box = new QWidget();
	QFormLayout *mem = new QFormLayout(mem_box);
	mem->setVerticalSpacing(10);
	mem->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
	m_elwro = new QSpinBox();
	m_elwro->setRange(0, 16);
	connect(m_elwro, &QSpinBox::valueChanged, this, [this](int v) {
		if (machine) machine->cfg.mem.elwro_modules = v;
		update_mem_sizes();
	});
	m_elwro_size = new QLabel();
	QHBoxLayout *elwro_row = new QHBoxLayout();
	elwro_row->setSpacing(10);
	elwro_row->addWidget(m_elwro);
	elwro_row->addWidget(m_elwro_size);
	elwro_row->addStretch(1);
	mem->addRow(tr("Elwro modules:"), elwro_row);
	m_mega = new QSpinBox();
	m_mega->setRange(0, 16);
	connect(m_mega, &QSpinBox::valueChanged, this, [this](int v) {
		if (machine) machine->cfg.mem.mega_modules = v;
		update_mem_sizes();
	});
	m_mega_size = new QLabel();
	QHBoxLayout *mega_row = new QHBoxLayout();
	mega_row->setSpacing(10);
	mega_row->addWidget(m_mega);
	mega_row->addWidget(m_mega_size);
	mega_row->addStretch(1);
	mem->addRow(tr("MEGA modules:"), mega_row);
	m_os_segments = new QSpinBox();
	m_os_segments->setRange(1, 2);
	connect(m_os_segments, &QSpinBox::valueChanged, this, [this](int v) {
		if (machine) machine->cfg.mem.os_segments = v;
		update_mem_sizes();
	});
	m_os_segments_size = new QLabel();
	QHBoxLayout *os_segments_row = new QHBoxLayout();
	os_segments_row->setSpacing(10);
	os_segments_row->addWidget(m_os_segments);
	os_segments_row->addWidget(m_os_segments_size);
	os_segments_row->addStretch(1);
	mem->addRow(tr("Hardwired OS pages:"), os_segments_row);

	m_mega_prom = new QLineEdit();
	m_mega_prom->setMinimumWidth(280);
	connect(m_mega_prom, &QLineEdit::editingFinished, this, [this]() {
		if (machine) set_cstr(&machine->cfg.mem.mega_prom_image, m_mega_prom->text());
	});
	QPushButton *prom_browse = new QPushButton(tr("Browse..."));
	connect(prom_browse, &QPushButton::clicked, this, [this]() {
		QString f = QFileDialog::getOpenFileName(this, tr("MEGA PROM image"), m_mega_prom->text());
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
		QString f = QFileDialog::getOpenFileName(this, tr("Preload program"), m_preload->text());
		if (!f.isNull()) m_preload->setText(f);
	});
	QHBoxLayout *preload_row = new QHBoxLayout();
	preload_row->addWidget(m_preload, 1);
	preload_row->addWidget(preload_browse);
	mem->addRow(tr("Preload program:"), preload_row);
	for (QWidget *w : std::initializer_list<QWidget *>{m_elwro, m_elwro_size, m_mega, m_mega_size, m_os_segments, m_os_segments_size, m_mega_prom,
			prom_browse, m_preload, preload_browse}) {
		gate(w, "cold");
	}
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
		return tr("SP45DE (8\" floppies)");
	case EM400_DEV_WINCHESTER:
		return tr("Winchester");
	case EM400_DEV_FLOP5:
		return tr("Floppy drive (5.25\")");
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
	gate(io_chan_num, "cold");
	chan_form->addRow(tr("Channel number:"), io_chan_num);
	io_chan_type = new QComboBox();
	io_chan_type->addItem(chan_type_label(EM400_CHANNEL_CHAR), EM400_CHANNEL_CHAR);
	io_chan_type->addItem(chan_type_label(EM400_CHANNEL_MULTIX), EM400_CHANNEL_MULTIX);
	io_chan_type->addItem(chan_type_label(EM400_CHANNEL_IOTESTER), EM400_CHANNEL_IOTESTER);
	connect(io_chan_type, &QComboBox::currentIndexChanged, this, [this]() {
		if (io_sel_chan < 0 || !machine) return;
		struct em400_channel_cfg *chan = &machine->cfg.channel[io_sel_chan];
		chan->type = (enum em400_channel_types) io_chan_type->currentData().toInt();
		// the new channel type may have fewer slots and take different device types
		for (int d=0 ; d<EM400_CHAN_MAX_DEV ; d++) {
			if ((d < em400_channel_max_devices(chan->type)) && em400_channel_dev_compatible(chan->type, chan->device[d].type)) {
				continue;
			}
			free_device_strings(&chan->device[d]);
			chan->device[d] = (struct em400_device_cfg){};
		}
		io_build_tree();
		io_update_buttons();
	});
	gate(io_chan_type, "cold");
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
	gate(io_dev_num, "cold");
	dev_form->addRow(tr("Device number:"), io_dev_num);
	io_dev_type = new QComboBox();
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
	gate(io_dev_type, "cold");
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
void ConfigDialog::io_populate_dev_types(int chan_type)
{
	QSignalBlocker b(io_dev_type);
	io_dev_type->clear();
	for (int t=EM400_DEV_NONE+1 ; t<EM400_DEV_TYPE_COUNT ; t++) {
		if (em400_channel_dev_compatible((enum em400_channel_types) chan_type, (enum em400_device_types) t)) {
			io_dev_type->addItem(dev_type_label(t), t);
		}
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
		int max = em400_channel_max_devices(chan->type);
		for (int i=0 ; i<max ; i++) {
			if (i == io_sel_dev || chan->device[i].type == EM400_DEV_NONE) {
				io_dev_num->addItem(QString::number(i), i);
			}
		}
		io_dev_num->setCurrentIndex(io_dev_num->findData(io_sel_dev));

		io_populate_dev_types(chan->type);
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
	bool dev_room = false;
	if (have_machine && io_sel_chan >= 0) {
		const struct em400_channel_cfg *chan = &machine->cfg.channel[io_sel_chan];
		int max = em400_channel_max_devices(chan->type);
		for (int d=0 ; d<max ; d++) {
			if (chan->device[d].type == EM400_DEV_NONE) {
				dev_room = true;
				break;
			}
		}
	}

	bool off = !ctl->is_powered();
	io_add_chan_btn->setEnabled(off && have_machine && !chan_full);
	io_add_dev_btn->setEnabled(off && io_sel_chan >= 0 && dev_room);
	io_remove_btn->setEnabled(off && io_sel_chan >= 0);
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
	enum em400_device_types def_type = EM400_DEV_NONE;
	for (int t=EM400_DEV_NONE+1 ; t<EM400_DEV_TYPE_COUNT ; t++) {
		if (em400_channel_dev_compatible(chan->type, (enum em400_device_types) t)) {
			def_type = (enum em400_device_types) t;
			break;
		}
	}
	if (def_type == EM400_DEV_NONE) return;

	int max = em400_channel_max_devices(chan->type);
	for (int d=0 ; d<max ; d++) {
		if (chan->device[d].type == EM400_DEV_NONE) {
			chan->device[d].type = def_type;
			if (def_type == EM400_DEV_TERMINAL) {
				chan->device[d].terminal.speed = 9600;
			}
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
			QString f = QFileDialog::getOpenFileName(this, caption, edit->text());
			if (!f.isNull()) edit->setText(f);
		});
		gate(edit, "cold");
		gate(browse, "cold");
		QHBoxLayout *row = new QHBoxLayout();
		row->addWidget(edit, 1);
		row->addWidget(browse);
		form->addRow(label, row);
	};

	auto disk_image_row = [this, form, ch, d](const QString &label, int slot, const char *cur, const QString &caption) {
		QLineEdit *edit = new QLineEdit();
		edit->setText(cur ? QString(cur) : QString());
		auto commit = [this, ch, d, slot, edit]() {
			appcfg_set_image(machine, ch, d, slot, edit->text().toUtf8().constData());
		};
		connect(edit, &QLineEdit::editingFinished, this, commit);
		QPushButton *browse = new QPushButton(tr("Browse..."));
		connect(browse, &QPushButton::clicked, this, [this, edit, caption, commit]() {
			QString f = QFileDialog::getOpenFileName(this, caption, edit->text());
			if (!f.isNull()) {
				edit->setText(f);
				commit();
			}
		});
		QString g = QStringLiteral("media:%1:%2:%3").arg(ch).arg(d).arg(slot);
		gate(edit, g);
		gate(browse, g);
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
		gate(port, "cold");
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
		gate(speed, "cold");
		form->addRow(tr("Speed (baud):"), speed);
		break;
	}
	case EM400_DEV_WINCHESTER:
		disk_image_row(tr("Disk image:"), 0, dev->winchester.image, tr("Winchester disk image"));
		break;
	case EM400_DEV_RTCLOCK:
		image_row(tr("PROM image:"), &dev->rtclock.prom, tr("RTC PROM image"));
		break;
	case EM400_DEV_SP45DE:
		form->addRow(new QLabel(tr("Slot images:")));
		for (int s=0 ; s<EM400_SP45DE_SLOT_COUNT ; s++) {
			disk_image_row(QStringLiteral("%1:").arg(s), s, dev->sp45de.images[s], tr("Floppy image for slot %1").arg(s));
		}
		break;
	default:
		break;
	}

	update_enabled_states();
}

// -----------------------------------------------------------------------
QWidget *ConfigDialog::build_log_page()
{
	QWidget *page = new QWidget();
	QFormLayout *form = new QFormLayout(page);
	form->setVerticalSpacing(10);

	QCheckBox *enabled = new QCheckBox(tr("Logging enabled"));
	enabled->setChecked(work.log.enabled);
	gate(enabled, "live");
	form->addRow(QString(), enabled);

	QWidget *config_box = new QWidget();
	QFormLayout *config_form = new QFormLayout(config_box);
	config_form->setVerticalSpacing(10);
	config_form->setContentsMargins(0, 0, 0, 0);
	connect(enabled, &QCheckBox::toggled, this, [this](bool on) {
		work.log.enabled = on;
	});

	QSet<int> on;
	QString comps = work.log.components ? QString(work.log.components) : QString();
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
	on.insert(L_LIB); // the core always keeps the library/error bucket on

	QGroupBox *comp_box = new QGroupBox(tr("Components"));
	QVBoxLayout *comp_layout = new QVBoxLayout(comp_box);
	QGridLayout *comp_grid = new QGridLayout();
	for (int i=1 ; i<L_COUNT ; i++) {
		QCheckBox *cb = new QCheckBox(QString(em400_log_component_name(i)));
		cb->setProperty("compid", i);
		cb->setChecked(on.contains(i));
		if (i == L_LIB) cb->setEnabled(false);
		else gate(cb, "live");
		connect(cb, &QCheckBox::toggled, this, [this]() { rebuild_log_components(); });
		comp_grid->addWidget(cb, (i - 1) / 3, (i - 1) % 3);
		m_log_components.append(cb);
	}
	comp_layout->addLayout(comp_grid);

	QHBoxLayout *sel_row = new QHBoxLayout();
	QPushButton *sel_all = new QPushButton(tr("Select all"));
	QPushButton *sel_none = new QPushButton(tr("Select none"));
	gate(sel_all, "live");
	gate(sel_none, "live");
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
	file->setText(work.log.file ? QString(work.log.file) : QString());
	connect(file, &QLineEdit::editingFinished, this, [this, file]() {
		set_cstr((const char **) &work.log.file, file->text());
	});
	QPushButton *file_browse = new QPushButton(tr("Browse..."));
	connect(file_browse, &QPushButton::clicked, this, [this, file]() {
		QString f = QFileDialog::getSaveFileName(this, tr("Log file"));
		if (!f.isNull()) file->setText(f);
	});
	gate(file, "live");
	gate(file_browse, "live");
	QHBoxLayout *file_row = new QHBoxLayout();
	file_row->addWidget(file, 1);
	file_row->addWidget(file_browse);
	config_form->addRow(tr("Log file:"), file_row);

	QCheckBox *line_buffered = new QCheckBox(tr("Line buffered"));
	line_buffered->setChecked(work.log.line_buffered);
	connect(line_buffered, &QCheckBox::toggled, this, [this](bool on) {
		work.log.line_buffered = on;
	});
	gate(line_buffered, "live");
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
	set_cstr((const char **) &work.log.components, names.join(','));
}

// -----------------------------------------------------------------------
void ConfigDialog::apply_log_live()
{
	if (work.log.file && *work.log.file) {
		em400_log_reopen(work.log.file,
			work.log.line_buffered ? EM400_LOG_LINE_BUFFERED : EM400_LOG_FULL_BUFFERED);
	}
	for (QCheckBox *cb : m_log_components) {
		em400_log_component_set(cb->property("compid").toUInt(), cb->isChecked());
	}
	em400_log_set(work.log.enabled);
}

// -----------------------------------------------------------------------
// One pass over every gated control: enabled iff the change can take effect
// now. Powered off -> everything cold materializes at power-on; powered on ->
// only live fields and ejectable media stay editable. Machine-block controls
// also require an active machine to edit. See gate().
void ConfigDialog::update_enabled_states()
{
	bool off = !ctl->is_powered();
	bool have = machine != nullptr;

	for (QWidget *w : findChildren<QWidget *>()) {
		QString g = w->property("gate").toString();
		if (g.isEmpty()) continue;

		bool enable;
		if (g == "live") {
			enable = true;
		} else if (g == "cold") {
			enable = off;
		} else if (g.startsWith("media:")) {
			const QStringList p = g.mid(6).split(':');
			enable = off || em400_dev_can_eject(p[0].toInt(), p[1].toInt(), p[2].toInt());
		} else {
			continue;
		}

		if (!have && machine_page->isAncestorOf(w)) enable = false;
		w->setEnabled(enable);
	}

	m_add_machine->setEnabled(off);
	m_dup_machine->setEnabled(off && have);
	// zero machines is not a valid state: power-on has nothing to run and the
	// saved file would round-trip through the legacy import at the next launch
	bool last = work.n_machines <= 1;
	m_del_machine->setEnabled(off && have && !last);
	m_del_machine->setToolTip(last ? tr("The last machine cannot be deleted.") : QString());

	io_update_buttons();
}

// -----------------------------------------------------------------------
void ConfigDialog::on_media_changed(unsigned chan, unsigned dev, unsigned slot, QString path)
{
	struct appcfg_machine *wm = appcfg_machine_find(&work, appcfg.active_id);
	if (!wm) return;

	appcfg_set_image(wm, chan, dev, slot, path.toUtf8().constData());

	if ((wm == machine) && (io_sel_chan == (int)chan) && (io_sel_dev == (int)dev)) {
		io_rebuild_dev_params();
	}
}

// -----------------------------------------------------------------------
void ConfigDialog::on_active_machine_changed(QString id)
{
	int idx = m_active->findData(id);
	if (idx >= 0) m_active->setCurrentIndex(idx); // cascades into slot_active_machine_changed
}

// -----------------------------------------------------------------------
QString ConfigDialog::unique_machine_id(const QString &name)
{
	QString base = derive_machine_id(name);
	QString id = base;
	for (int n=2 ; appcfg_machine_find(&work, id.toUtf8().constData()) ; n++) {
		id = base + QString::number(n);
	}
	return id;
}

// -----------------------------------------------------------------------
bool ConfigDialog::prompt_machine_identity(const QString &title, const QString &initial_name, QString &name, QString &id)
{
	QDialog dlg(this);
	dlg.setWindowTitle(title);
	QFormLayout *form = new QFormLayout(&dlg);
	form->setVerticalSpacing(10);

	QLineEdit *name_edit = new QLineEdit(initial_name);
	name_edit->setMinimumWidth(220);
	name_edit->selectAll();
	form->addRow(tr("Name:"), name_edit);

	QLabel *id_label = new QLabel();
	id_label->setEnabled(false);
	id_label->setToolTip(tr("Derived from the name, fixed at machine creation.\nIdentifies the machine in the configuration file\nand for the -m command line option."));
	form->addRow(tr("Identifier:"), id_label);

	auto update_id = [this, name_edit, id_label]() {
		id_label->setText(unique_machine_id(name_edit->text()));
	};
	connect(name_edit, &QLineEdit::textChanged, &dlg, update_id);
	update_id();

	QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
	form->addRow(buttons);

	if (dlg.exec() != QDialog::Accepted) return false;
	name = name_edit->text().trimmed();
	id = unique_machine_id(name_edit->text());
	return true;
}

// -----------------------------------------------------------------------
void ConfigDialog::machine_add(bool duplicate)
{
	if (duplicate && !machine) return;

	QString initial;
	if (duplicate) {
		initial = tr("%1 (copy)").arg(machine->name ? QString(machine->name) : QString(machine->id));
	}
	QString src_id = machine ? QString(machine->id) : QString();

	QString name, id;
	QString title = duplicate ? tr("Duplicate machine") : tr("Add machine");
	if (!prompt_machine_identity(title, initial, name, id)) return;

	QByteArray id8 = id.toUtf8();
	QByteArray name8 = name.toUtf8();
	const char *cname = name.isEmpty() ? nullptr : name8.constData();
	struct appcfg_machine *m;
	if (duplicate) {
		m = appcfg_machine_clone(&work, src_id.toUtf8().constData(), id8.constData(), cname);
	} else {
		m = appcfg_machine_add_default(&work, id8.constData(), cname);
	}
	if (!m) return;

	// adding may realloc work.machines: `machine` is stale until the combo change
	// cascades into slot_active_machine_changed, which re-resolves it
	m_active->addItem(name.isEmpty() ? id : name, id);
	m_active->setCurrentIndex(m_active->count() - 1);
}

// -----------------------------------------------------------------------
void ConfigDialog::machine_delete()
{
	if (!machine) return;

	QString label = machine->name ? QString(machine->name) : QString(machine->id);
	if (QMessageBox::question(this, tr("Delete machine"), tr("Delete machine \"%1\"?").arg(label)) != QMessageBox::Yes) {
		return;
	}

	QByteArray id8(machine->id);
	machine = nullptr; // dangling after the delete; the combo cascade below re-resolves it
	appcfg_machine_delete(&work, id8.constData());
	m_active->removeItem(m_active->currentIndex());
}

// -----------------------------------------------------------------------
void ConfigDialog::reload_machine_page()
{
	bool have = machine != nullptr;

	io_sel_chan = io_sel_dev = -1;
	if (io_tree) {
		io_build_tree();
		io_editor->setCurrentIndex(0);
		io_update_buttons();
	}

	update_enabled_states();

	if (!have) {
		m_id->clear();
		return;
	}

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
	m_id->setText(QString(machine->id));
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
	update_mem_sizes();
}

// -----------------------------------------------------------------------
void ConfigDialog::update_mem_sizes()
{
	m_elwro_size->setText(tr("(%n kwords)", nullptr, m_elwro->value() * 32));
	m_mega_size->setText(tr("(%n kwords)", nullptr, m_mega->value() * 64));
	m_os_segments_size->setText(tr("(%n kwords)", nullptr, m_os_segments->value() * 4));
}

// -----------------------------------------------------------------------
void ConfigDialog::accept_config()
{
	// the file write is the commit point: on failure nothing is applied or
	// persisted, so Cancel stays meaningful and the dialog can be retried
	if (!ctl->apply_and_save(&work)) {
		QMessageBox::warning(this, tr("Settings"), tr("Failed to save the configuration file."));
		return;
	}
	apply_log_live();
	emit signal_machine_renamed(); // committed active machine name may have changed
	accept();
}

// -----------------------------------------------------------------------
void ConfigDialog::reject()
{
	ctl->preview_volume(orig_volume); // undo the live volume preview

	// the mono font applied live on each Change/Reset; restore what was set at open
	if (mono_font_touched) {
		QSettings s;
		if (orig_mono_font_set) {
			s.setValue("ui/monoFontFamily", orig_mono_font_family);
			s.setValue("ui/monoFontSize", orig_mono_font_size);
		} else {
			s.remove("ui/monoFontFamily");
			s.remove("ui/monoFontSize");
		}
		emit signal_mono_font_changed();
	}

	QDialog::reject();
}

// -----------------------------------------------------------------------
void ConfigDialog::slot_active_machine_changed(int index)
{
	(void) index;
	QString id = m_active->currentData().toString();
	set_cstr((const char **) &work.active_id, id);
	machine = appcfg_machine_find(&work, work.active_id);
	reload_machine_page();
}

// vim: tabstop=4 shiftwidth=4 autoindent

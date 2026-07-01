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

#include <QApplication>
#include <QStyle>
#include <QStyleFactory>
#include <QFont>
#include <QSettings>

#include "theme.h"

// -----------------------------------------------------------------------
void em400_apply_mono_font(QFont &f)
{
	f.setStyleHint(QFont::Monospace);

	QSettings s;
	const QString family = s.value(QStringLiteral("ui/monoFontFamily")).toString();
	if (!family.isEmpty()) {
		f.setFamily(family);
	} else {
		// no user choice: the platform's best default.
#if defined(Q_OS_WIN)
		f.setFamilies({QStringLiteral("Cascadia Mono"), QStringLiteral("Consolas")});
#elif defined(Q_OS_MACOS)
		f.setFamilies({QStringLiteral("Menlo")});
#else
		f.setFamily(QStringLiteral("Monospace"));
#endif
	}

	const int size = s.value(QStringLiteral("ui/monoFontSize"), 0).toInt();
	if (size > 0) f.setPointSize(size);
}

// The "MERA-400 LED" palette, derived by eye from the control-panel look.
// The accents are intensity-matched bright LED primaries (green/yellow/red)
// so they read as one family.
// - background + lettering are structural (the palette is built from them);
// - separator/green/yellow/red are the semantic accents
static const QColor c_background(0x38, 0x38, 0x38); // panel gunmetal
static const QColor c_lettering(0xeb, 0xeb, 0xe9); // near-white labels, ~9.8:1 on bg
static const QColor c_separator(0xab, 0x57, 0x4f); // muted red: divider/separator rules
static const QColor c_green(0x99, 0xf7, 0x00); // "you are here": Highlight, IC bar, edit cell
static const QColor c_yellow(0xff, 0xf3, 0x25); // "this is on": mask boxes, allocation map, pending int
static const QColor c_red(0xff, 0x42, 0x61); // "won't execute": P-flag IC bar

// Whether the panel theme is currently applied (kept in sync by
// em400_apply_theme). Defaults to the startup default (panel on).
static bool g_panel_active = true;

// -----------------------------------------------------------------------
QPalette em400_panel_palette()
{
	// The two structural colors: the gunmetal background (from which all the
	// Fusion shade roles are derived by lightness) and the off-white lettering.
	const QColor bg = c_background;
	const QColor text = c_lettering;
	const QColor green = c_green;

	// Shade roles derived from the background so bevels/borders track the chosen
	// gunmetal. lighter()/darker() take a percentage (>100 = lighter/darker).
	const QColor c_base = bg.darker(135);
	const QColor c_altbase = bg.darker(112);
	const QColor c_button = bg.lighter(118);
	const QColor c_light = bg.lighter(150);
	const QColor c_midlight = bg.lighter(110);
	const QColor c_mid = bg.lighter(165);
	const QColor c_dark = bg.darker(140);
	const QColor c_shadow = bg.darker(280);
	const QColor c_tooltip = bg.darker(150);
	// dim/ghost text: the lettering pulled well down toward the background
	const QColor c_disabled = text.darker(180);
	// dark text that rides on the green highlight fill
	const QColor c_hltext = green.darker(600);

	QPalette p;

	p.setColor(QPalette::Window, bg);
	p.setColor(QPalette::WindowText, text);
	p.setColor(QPalette::Base, c_base);
	p.setColor(QPalette::AlternateBase, c_altbase);
	p.setColor(QPalette::Text, text);
	p.setColor(QPalette::Button, c_button);
	p.setColor(QPalette::ButtonText, text);
	p.setColor(QPalette::BrightText, Qt::white);
	p.setColor(QPalette::ToolTipBase, c_tooltip);
	p.setColor(QPalette::ToolTipText, text);
	p.setColor(QPalette::PlaceholderText, c_disabled);
	// Shade roles kept dark-relative so Fusion bevels/borders read correctly on
	// the dark window (these carry their normal Qt meaning - not repurposed).
	p.setColor(QPalette::Light, c_light);
	p.setColor(QPalette::Midlight, c_midlight);
	p.setColor(QPalette::Mid, c_mid);
	p.setColor(QPalette::Dark, c_dark);
	p.setColor(QPalette::Shadow, c_shadow);
	p.setColor(QPalette::Highlight, green);
	p.setColor(QPalette::HighlightedText, c_hltext);
	p.setColor(QPalette::Link, green);

	// Dim/ghost text used by IntView/MapView/RegCompact comes from the
	// Disabled group's WindowText/Text role.
	p.setColor(QPalette::Disabled, QPalette::WindowText, c_disabled);
	p.setColor(QPalette::Disabled, QPalette::Text, c_disabled);
	p.setColor(QPalette::Disabled, QPalette::ButtonText, c_disabled);

	return p;
}

// -----------------------------------------------------------------------
void em400_apply_theme(bool panel)
{
	// Capture the application's original style name and palette on first call,
	// so toggling the theme off can restore the native look.
	static bool captured = false;
	static QString default_style;
	static QPalette default_palette;

	if (!captured) {
		if (qApp->style()) default_style = qApp->style()->name();
		default_palette = qApp->palette();
		captured = true;
	}

	g_panel_active = panel;

	if (panel) {
		qApp->setStyle(QStyleFactory::create("Fusion"));
		qApp->setPalette(em400_panel_palette());
		// Fusion's default separator rule is darker than the gunmetal menu and
		// nearly invisible on it; lift it a touch above the background instead.
		const QColor sep = c_background.lighter(155);
		qApp->setStyleSheet(QString("QMenu::separator { height: 1px; background: %1; margin: 4px 8px; }").arg(sep.name()));
	} else {
		if (!default_style.isEmpty()) {
			qApp->setStyle(QStyleFactory::create(default_style));
		}
		qApp->setPalette(default_palette);
		qApp->setStyleSheet(QString());
	}
}

// -----------------------------------------------------------------------
bool em400_theme_is_panel()
{
	return g_panel_active;
}

// -----------------------------------------------------------------------
// Divider/separator rules: muted red in the panel theme; the palette Mid grey
// (the natural divider shade) under the system theme.
QColor em400_sep_color(const QPalette &pal)
{
	return g_panel_active ? c_separator : pal.color(QPalette::Mid);
}

// -----------------------------------------------------------------------
// Secondary text that must stay clearly readable while still reading as
// "less primary" than the main Text (used for the memory view ascii/r40
// panel next to the full-contrast hex/dec values). PlaceholderText/Disabled
// are deliberately too dim for this, so instead blend the theme's real Text
// toward Base by a fixed fraction. Because it interpolates between the actual
// foreground and background of whichever theme is active, it lands at a
// consistent mid-high contrast in both dark and light modes.
QColor em400_dim_text_color(const QPalette &pal)
{
	const QColor text = pal.color(QPalette::Text);
	const QColor base = pal.color(QPalette::Base);
	const qreal t = 0.30; // 0 = full Text contrast, 1 = invisible
	return QColor(
		qRound(text.red() * (1.0 - t) + base.red() * t),
		qRound(text.green() * (1.0 - t) + base.green() * t),
		qRound(text.blue() * (1.0 - t) + base.blue() * t));
}

// -----------------------------------------------------------------------
// Amber accent used by the interrupt mask boxes and the allocation map: the
// yellow slot in the panel theme; the desktop accent (palette Highlight) under
// the system theme.
QColor em400_mask_color(const QPalette &pal)
{
	return g_panel_active ? c_yellow : pal.color(QPalette::Highlight);
}

// -----------------------------------------------------------------------
// Red accent - "will not take effect": something is present but inert/rejected.
// Two uses, treated as one signal to the user: the emulation state (the IC-bar
// instruction will NOT execute when P is set) and UI feedback (a rejected
// breakpoint expression that won't run). The red slot in the panel theme; under
// the system theme keep the previous look (a lightened Qt red) since there is no
// red palette role.
QColor em400_red_color(const QPalette &pal)
{
	(void) pal;
	return g_panel_active ? c_red : QColor(Qt::red).lighter();
}

// vim: tabstop=4 shiftwidth=4 autoindent

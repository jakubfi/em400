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

#ifndef EM400_QT6_THEME_H
#define EM400_QT6_THEME_H

#include <QPalette>
#include <QColor>

class QFont;

// Make `f` the monospace font. See theme.cpp for the platform defaults.
void em400_apply_mono_font(QFont &f);

// Dark palette derived from the control-panel ("pulpit") artwork colors.
QPalette em400_panel_palette();

// Apply or remove the panel theme on the whole application. When `panel` is
// true the Fusion style + panel palette are forced; when false the original
// startup style/palette (captured on first call) are restored.
void em400_apply_theme(bool panel);

// True while the custom panel theme is active (false under the user's system
// theme). Lets the semantic color getters below pick brand vs system colors.
bool em400_theme_is_panel();

// Semantic UI colors that stay correct under BOTH the panel theme and the
// user's desktop theme. In the panel theme they return the distinctive
// control-panel brand colors; under the system theme they fall back to the
// palette role that carries the same meaning there, so nothing is hardcoded
// against a single theme. Pass the consuming widget's palette() for the
// system fallback.
QColor em400_sep_color(const QPalette &pal); // divider / separator rules
QColor em400_dim_text_color(const QPalette &pal); // secondary-but-readable text (ascii/r40 panel)
QColor em400_mask_color(const QPalette &pal); // mask boxes + allocation map (amber)
QColor em400_red_color(const QPalette &pal); // "will not take effect": P-flag IC bar + rejected input (red)

#endif // EM400_QT6_THEME_H

// vim: tabstop=4 shiftwidth=4 autoindent

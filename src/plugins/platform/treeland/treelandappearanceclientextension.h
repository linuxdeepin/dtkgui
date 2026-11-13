// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TREELANDAPPEARANCECLIENTEXTENSION_H
#define TREELANDAPPEARANCECLIENTEXTENSION_H

#include "dtreelandplatforminterface.h"
#include "qwayland-treeland-appearance-unstable-v1.h"

#include <dtkgui_global.h>

#include <QtWaylandClient/QWaylandClientExtension>
#include <QWaylandClientExtensionTemplate>

DGUI_BEGIN_NAMESPACE

class DTreelandPlatformInterfacePrivate;

class TreelandAppearance : public QWaylandClientExtensionTemplate<TreelandAppearance>,
                          public QtWayland::treeland_appearance_v1
{
    Q_OBJECT
public:
    explicit TreelandAppearance(DTreelandPlatformInterface *interface);
    ~TreelandAppearance() override;

protected:
    void treeland_appearance_v1_cursor_theme(const QString &name) override;
    void treeland_appearance_v1_cursor_size(uint32_t width, uint32_t height) override;
    void treeland_appearance_v1_font(const QString &font_name) override;
    void treeland_appearance_v1_monospace_font(const QString &font_name) override;
    void treeland_appearance_v1_font_size(uint32_t size) override;
    void treeland_appearance_v1_icon_theme(const QString &theme_name) override;
    void treeland_appearance_v1_accent_color(uint32_t r, uint32_t g, uint32_t b) override;
    void treeland_appearance_v1_window_opacity(wl_fixed_t opacity) override;
    void treeland_appearance_v1_color_scheme(uint32_t scheme) override;
    void treeland_appearance_v1_titlebar_height(uint32_t height) override;
    void treeland_appearance_v1_corner_radius(uint32_t radius) override;

private:
    DTreelandPlatformInterface *m_interface;
};

DGUI_END_NAMESPACE

#endif // TREELANDAPPEARANCECLIENTEXTENSION_H

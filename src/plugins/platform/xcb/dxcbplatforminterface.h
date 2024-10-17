// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DXCBPLATFORMINTERFACE_H
#define DXCBPLATFORMINTERFACE_H

#include "plugins/dplatforminterface.h"

DGUI_BEGIN_NAMESPACE

class DXCBPlatformInterfacePrivate;
class DXCBPlatformInterface : public DPlatformInterface
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DXCBPlatformInterface)
public:
    explicit DXCBPlatformInterface(quint32 window, QObject *parent);

    bool isEnabledNoTitlebar(QWindow *window) const override;
    bool setEnabledNoTitlebar(QWindow *window, bool enable) override;

    int windowRadius() const override;
    void setWindowRadius(int windowRadius) override;

    QByteArray fontName() const override;
    void setFontName(const QByteArray &fontName) override;

    QByteArray monoFontName() const override;
    void setMonoFontName(const QByteArray &monoFontName) override;

    QByteArray iconThemeName() const override;
    void setIconThemeName(const QByteArray &iconThemeName) override;

    QByteArray cursorThemeName() const override;
    void setCursorThemeName(const QByteArray &cursorThemeName) override;
};

DGUI_END_NAMESPACE
#endif

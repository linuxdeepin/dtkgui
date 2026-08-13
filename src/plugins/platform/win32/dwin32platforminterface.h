// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DWIN32PLATFORMINTERFACE_H
#define DWIN32PLATFORMINTERFACE_H

#include "private/dplatforminterface_p.h"

DGUI_BEGIN_NAMESPACE

class DWin32PlatformInterface : public QObject, public DPlatformInterface
{
    Q_OBJECT
public:
    explicit DWin32PlatformInterface(DPlatformTheme *platformTheme);

    QByteArray themeName() const override;
    QByteArray iconThemeName() const override;
    QByteArray soundThemeName() const override;

    QByteArray fontName() const override;
    QByteArray monoFontName() const override;
    qreal fontPointSize() const override;
    QByteArray gtkFontName() const override;

    QColor activeColor() const override;
    QColor darkActiveColor() const override;

    int windowRadius() const override;
    int windowRadius(int defaultValue) const override;

    void setThemeName(const QByteArray &themeName) override;
    void setIconThemeName(const QByteArray &iconThemeName) override;
    void setSoundThemeName(const QByteArray &soundThemeName) override;
    void setFontName(const QByteArray &fontName) override;
    void setMonoFontName(const QByteArray &monoFontName) override;
    void setFontPointSize(qreal fontPointSize) override;
    void setGtkFontName(const QByteArray &fontName) override;
    void setActiveColor(const QColor activeColor) override;
    void setDarkActiveColor(const QColor &activeColor) override;
    void setWindowRadius(int windowRadius) override;

private:
    mutable QByteArray m_iconThemeName;
    mutable QByteArray m_fontName;
    mutable QByteArray m_monoFontName;
    mutable QByteArray m_gtkFontName;
    mutable QColor m_activeColor;
    mutable QColor m_darkActiveColor;
};

DGUI_END_NAMESPACE
#endif // DWIN32PLATFORMINTERFACE_H

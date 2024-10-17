// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLATFORMINTERFACE_H
#define DPLATFORMINTERFACE_H

#include <QObject>
#include <QVariant>
#include <QWindow>

#include "dtkgui_global.h"
#include <dobject.h>

DGUI_BEGIN_NAMESPACE

class DPlatformInterfacePrivate;
class DPlatformInterface : public QObject, public DCORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DPlatformInterface)
public:
    explicit DPlatformInterface(QObject *parent);
    virtual ~DPlatformInterface();

    virtual bool isEnabledNoTitlebar(QWindow *window) const;
    virtual bool setEnabledNoTitlebar(QWindow *window, bool enable);

    virtual int windowRadius() const;
    virtual void setWindowRadius(int windowRadius);

    virtual QByteArray fontName() const;
    virtual void setFontName(const QByteArray &fontName);

    virtual QByteArray monoFontName() const;
    virtual void setMonoFontName(const QByteArray &monoFontName);

    virtual QByteArray iconThemeName() const;
    virtual void setIconThemeName(const QByteArray &iconThemeName);

    virtual QByteArray cursorThemeName() const;
    virtual void setCursorThemeName(const QByteArray &cursorThemeName);

    static DPlatformInterface* self(QObject *parent);

protected:
    DPlatformInterface(DPlatformInterfacePrivate &dd, QObject *parent);
};

DGUI_END_NAMESPACE
#endif

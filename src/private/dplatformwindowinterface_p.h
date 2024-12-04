// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLATFORMWINDOWINTERFACE_P_H
#define DPLATFORMWINDOWINTERFACE_P_H

#include <QObject>
#include <DObject>

#include "dtkgui_global.h"
#include "dplatformhandle.h"

DGUI_BEGIN_NAMESPACE

class DPlatformWindowInterfacePrivate;
class LIBDTKCORESHARED_EXPORT DPlatformWindowInterface : public QObject, public DCORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DPlatformWindowInterface)
public:
    explicit DPlatformWindowInterface(QWindow *window, DPlatformHandle *platformHandle, QObject *parent=nullptr);
    virtual ~DPlatformWindowInterface();

    QWindow* window() const;

    virtual bool setWindowBlurArea(const QVector<DPlatformHandle::WMBlurArea> &area);
    virtual bool setWindowBlurArea(const QList<QPainterPath> &paths);
    virtual bool setWindowWallpaperPara(const QRect &area, DPlatformHandle::WallpaperScaleMode sMode, DPlatformHandle::WallpaperFillMode fMode);

    virtual bool isEnabledNoTitlebar() const;
    virtual bool setEnabledNoTitlebar(bool enable);

    virtual void setDisableWindowOverrideCursor(bool disable);

    virtual int windowRadius() const;
    virtual void setWindowRadius(int windowRadius);

    virtual int borderWidth() const;
    virtual void setBorderWidth(int borderWidth);
 
    virtual QColor borderColor() const;
    virtual void setBorderColor(const QColor &borderColor);

    virtual int shadowRadius() const;
    virtual void setShadowRadius(int shadowRadius);

    virtual QPoint shadowOffset() const;
    virtual void setShadowOffset(const QPoint &shadowOffset);

    virtual QColor shadowColor() const;
    virtual void setShadowColor(const QColor &shadowColor);

    virtual DPlatformHandle::EffectScene windowEffect();
    virtual void setWindowEffect(DPlatformHandle::EffectScenes effectScene);

    virtual DPlatformHandle::EffectType windowStartUpEffect();
    virtual void setWindowStartUpEffect(DPlatformHandle::EffectTypes effectType);

    virtual QPainterPath clipPath() const;
    virtual void setClipPath(const QPainterPath &clipPath);

    virtual QRegion frameMask() const;
    virtual void setFrameMask(const QRegion &frameMask);

    virtual QMargins frameMargins() const;

    virtual bool translucentBackground() const;
    virtual void setTranslucentBackground(bool translucentBackground);

    virtual bool enableSystemResize() const;
    virtual void setEnableSystemResize(bool enableSystemResize);

    virtual bool enableSystemMove() const;
    virtual void setEnableSystemMove(bool enableSystemMove);

    virtual bool enableBlurWindow() const;
    virtual void setEnableBlurWindow(bool enableBlurWindow);

    virtual bool autoInputMaskByClipPath() const;
    virtual void setAutoInputMaskByClipPath(bool autoInputMaskByClipPath);

    virtual WId realWindowId() const;

protected:
    DPlatformWindowInterface(DPlatformWindowInterfacePrivate &dd, QWindow *window, DPlatformHandle *handle, QObject *parent);
};

class LIBDTKCORESHARED_EXPORT DPlatformWindowInterfaceFactory {
public:
    using HelperCreator = DPlatformWindowInterface * (*)(QWindow *, DPlatformHandle*);
    static void registerInterface(HelperCreator creator);
};

DGUI_END_NAMESPACE
#endif // DPLATFORMWINDOWINTERFACE_P_H


// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DXCBPLATFORMWINDOWINTERFACE_H
#define DXCBPLATFORMWINDOWINTERFACE_H

#include "private/dplatformwindowinterface_p.h"

#include <QWindow>

DGUI_BEGIN_NAMESPACE

class DXCBPlatformWindowInterfacePrivate;
class DXCBPlatformWindowInterface : public DPlatformWindowInterface
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DXCBPlatformWindowInterface)
public:
    explicit DXCBPlatformWindowInterface(QWindow *window, DPlatformHandle *platformHandle, QObject *parent);
    ~DXCBPlatformWindowInterface();

    static QString pluginVersion();
    static bool isDXcbPlatform();
    static bool connectWindowManagerChangedSignal(QObject *object, std::function<void ()> slot);
    static bool connectHasBlurWindowChanged(QObject *object, std::function<void ()> slot);
    static WId windowLeader();

    void enableDXcb();
    void enableDXcb(bool redirectContent);
    bool isEnabledDXcb();
    bool eventFilterForXcb(QObject *obj, QEvent *event);

    bool setWindowBlurArea(const QVector<DPlatformHandle::WMBlurArea> &area) override;
    bool setWindowBlurArea(const QList<QPainterPath> &paths) override;
    bool setWindowWallpaperPara(const QRect &area, DPlatformHandle::WallpaperScaleMode sMode, DPlatformHandle::WallpaperFillMode fMode) override;

    void setDisableWindowOverrideCursor(bool disable) override;

    bool isEnabledNoTitlebar() const override;
    bool setEnabledNoTitlebar(bool enable) override;

    int windowRadius() const override;
    void setWindowRadius(int windowRadius) override;

    int borderWidth() const override;
    void setBorderWidth(int borderWidth) override;

    QColor borderColor() const override;
    void setBorderColor(const QColor &borderColor) override;

    int shadowRadius() const override;
    void setShadowRadius(int shadowRadius) override;

    QPoint shadowOffset() const override;
    void setShadowOffset(const QPoint &shadowOffset) override;

    QColor shadowColor() const override;
    void setShadowColor(const QColor &shadowColor) override;

    DPlatformHandle::EffectScene windowEffect() override;
    void setWindowEffect(DPlatformHandle::EffectScenes effectScene) override;

    DPlatformHandle::EffectType windowStartUpEffect() override;
    void setWindowStartUpEffect(DPlatformHandle::EffectTypes effectType) override;

    QPainterPath clipPath() const override;
    void setClipPath(const QPainterPath &clipPath) override;

    QRegion frameMask() const override;
    void setFrameMask(const QRegion &frameMask) override;

    QMargins frameMargins() const override;

    bool translucentBackground() const override;
    void setTranslucentBackground(bool translucentBackground) override;

    bool enableSystemResize() const override;
    void setEnableSystemResize(bool enableSystemResize) override;

    bool enableSystemMove() const override;
    void setEnableSystemMove(bool enableSystemMove) override;

    bool enableBlurWindow() const override;
    void setEnableBlurWindow(bool enableBlurWindow) override;

    bool autoInputMaskByClipPath() const override;
    void setAutoInputMaskByClipPath(bool autoInputMaskByClipPath) override;

    WId realWindowId() const override;
};

DGUI_END_NAMESPACE
#endif // DXCBPLATFORMWINDOWINTERFACE_H

// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DXCBPLATFORMWINDOWINTERFACE_H
#define DXCBPLATFORMWINDOWINTERFACE_H

#include "private/dplatformwindowinterface_p.h"

#include <QWindow>

DGUI_BEGIN_NAMESPACE

class DXCBPlatformWindowInterface : public QObject, public DPlatformWindowInterface
{
    Q_OBJECT
public:
    DXCBPlatformWindowInterface(QWindow *window, DPlatformHandle *platformHandle, QObject *parent = nullptr);
    ~DXCBPlatformWindowInterface() override;

    static QString pluginVersion();
    static bool isDXcbPlatform();
    static bool connectWindowManagerChangedSignal(QObject *object, std::function<void ()> slot);
    static bool connectHasBlurWindowChanged(QObject *object, std::function<void ()> slot);
    static WId windowLeader();

    void enableDXcb(bool redirectContent);

    bool setWindowBlurArea(const QVector<DPlatformHandle::WMBlurArea> &area);
    bool setWindowBlurArea(const QList<QPainterPath> &paths);
    bool setWindowWallpaperPara(const QRect &area, DPlatformHandle::WallpaperScaleMode sMode, DPlatformHandle::WallpaperFillMode fMode);

    bool autoInputMaskByClipPath() const;
    void setAutoInputMaskByClipPath(bool autoInputMaskByClipPath);

    WId realWindowId() const;

    void setEnabled(bool enabled) override;
    bool isEnabled() const override;

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

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

DGUI_END_NAMESPACE
#endif // DXCBPLATFORMWINDOWINTERFACE_H

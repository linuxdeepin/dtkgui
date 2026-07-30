// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DWIN32PLATFORMWINDOWINTERFACE_H
#define DWIN32PLATFORMWINDOWINTERFACE_H

#include "private/dplatformwindowinterface_p.h"

#include <QWindow>

DGUI_BEGIN_NAMESPACE

class DWin32PlatformWindowInterface : public QObject, public DPlatformWindowInterface
{
    Q_OBJECT
public:
    DWin32PlatformWindowInterface(QWindow *window, DPlatformHandle *platformHandle, QObject *parent = nullptr);
    ~DWin32PlatformWindowInterface() override;

    void setEnabled(bool enabled) override;
    bool isEnabled() const override;

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

    void setDisableWindowOverrideCursor(bool disable) override;

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

private:
    void updateShadow();
    void updateRoundedCorners();
    void updateSystemTheme();
    void *hwnd() const;

    void updateClipPath();
    void updateFrameMask();
    void updateTranslucentBackground();
    void updateBlurWindow();

    int m_windowRadius = 12;
    int m_borderWidth = 1;
    QColor m_borderColor;
    int m_shadowRadius = -1;
    QPoint m_shadowOffset;
    QColor m_shadowColor;
    DPlatformHandle::EffectScenes m_windowEffect;
    DPlatformHandle::EffectTypes m_windowStartUpEffect;
    QPainterPath m_clipPath;
    QRegion m_frameMask;
    bool m_translucentBackground = false;
    bool m_enableSystemResize = true;
    bool m_enableSystemMove = true;
    bool m_enableBlurWindow = false;
    bool m_disableOverrideCursor = false;
    bool m_enabled = false;
    bool m_windowVisible = false;
    bool m_systemDarkMode = false;
    bool m_windowMoving = false;
    QPointF m_pressPoint;
};

DGUI_END_NAMESPACE
#endif // DWIN32PLATFORMWINDOWINTERFACE_H

// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTREELANDPLATFORMWINDOWINTERFACE_H
#define DTREELANDPLATFORMWINDOWINTERFACE_H

#include "dtkgui_global.h"
#include <QColor>
#include <QObject>
#include <QPainterPath>
#include <QRegion>
#include "private/dplatformwindowinterface_p.h"

DGUI_BEGIN_NAMESPACE
class TreelandDecorationContext;
class BackgroundEffectSurface;
class DTreeLandPlatformWindowHelper : public QObject {
    Q_OBJECT
public:
    static DTreeLandPlatformWindowHelper *get(QWindow *window);
    ~DTreeLandPlatformWindowHelper() override;

    enum Feature {
        NoTitlebar   = 1 << 0,
        Radius       = 1 << 1,
        Border       = 1 << 2,
        Shadow       = 1 << 3,
        WindowEffect = 1 << 4,
        BlurWindow   = 1 << 5,
        BlurRegion   = 1 << 6,
    };
    Q_DECLARE_FLAGS(Features, Feature)

    QWindow *window() const { return qobject_cast<QWindow *>(parent()); }
    TreelandDecorationContext *decorationContext() const;
    BackgroundEffectSurface *backgroundEffectSurface() const;

    void setEnabledNoTitlebar(bool enable);
    void setWindowRadius(int windowRadius);
    void setBorderWidth(int borderWidth);
    void setBorderColor(const QColor &borderColor);
    void setShadowRadius(int shadowRadius);
    void setShadowOffset(const QPoint &shadowOffset);
    void setShadowColor(const QColor &shadowColor);
    void setWindowEffect(DPlatformHandle::EffectScenes effectScene);
    void setPlatformHandle(DPlatformHandle *handle);
    void setEnableBlurWindow(bool enabled);
    void setWindowBlurAreas(const QVector<DPlatformHandle::WMBlurArea> &areas);
    void setWindowBlurPaths(const QList<QPainterPath> &paths);
    bool enableBlurWindow() const { return m_blurWindowEnabled; }

    bool isEnabledNoTitlebar() const { return m_noTitlebar; }
    int windowRadius() const { return m_radius; }
    int borderWidth() const { return m_borderWidth; }
    QColor borderColor() const { return m_borderColor; }
    int shadowRadius() const { return m_shadowRadius; }
    QPoint shadowOffset() const { return m_shadowOffset; }
    QColor shadowColor() const { return m_shadowColor; }
    DPlatformHandle::EffectScenes windowEffect() const { return m_effectScene; }

private slots:
    void onActiveChanged();
    void onSurfaceCreated();
    void onSurfaceDestroyed();
    void applyPending();
private:
    explicit DTreeLandPlatformWindowHelper(QWindow *window);
    bool eventFilter(QObject *watched, QEvent *event) override;
    void initWaylandWindow();
    void scheduleApply();

    template<typename T>
    void updateFeature(T &member, const T &value, Feature flag) {
        member = value;
        updateFeature(flag);
    }

    void updateFeature(Feature flag) {
        m_initialized |= flag;
        m_dirty |= flag;
        scheduleApply();
    }

private:
    TreelandDecorationContext *m_decorationContext = nullptr;
    BackgroundEffectSurface *m_backgroundEffectSurface = nullptr;
    static QMap<QWindow *, DTreeLandPlatformWindowHelper*> windowMap;
    DPlatformHandle *m_platformHandle = nullptr;

    bool m_noTitlebar = false;
    bool m_blurWindowEnabled = false;
    QRegion m_blurRegion;
    int m_radius = 0;
    int m_borderWidth = 0;
    QColor m_borderColor;
    int m_shadowRadius = 0;
    QPoint m_shadowOffset;
    QColor m_shadowColor;
    DPlatformHandle::EffectScenes m_effectScene = {};
    Features m_initialized = {};
    Features m_dirty = {};
    bool m_applyScheduled = false;
};

class DTreeLandPlatformWindowInterface : public QObject, public DPlatformWindowInterface
{
    Q_OBJECT
public:
    DTreeLandPlatformWindowInterface(QWindow *window, DPlatformHandle *platformHandle, QObject *parent = nullptr);
    ~DTreeLandPlatformWindowInterface() override;

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

    bool enableBlurWindow() const override;
    void setEnableBlurWindow(bool enableBlurWindow) override;

    DPlatformHandle::EffectScene windowEffect() override;
    void setWindowEffect(DPlatformHandle::EffectScenes effectScene) override;

    void setWindowBlurAreas(const QVector<DPlatformHandle::WMBlurArea> &areas);
    void setWindowBlurPaths(const QList<QPainterPath> &paths);
};

DGUI_END_NAMESPACE

#endif // DTREELANDPLATFORMWINDOWINTERFACE_H

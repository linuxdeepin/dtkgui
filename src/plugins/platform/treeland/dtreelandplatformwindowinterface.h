// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTREELANDPLATFORMWINDOWINTERFACE_H
#define DTREELANDPLATFORMWINDOWINTERFACE_H

#include "dtkgui_global.h"
#include <QColor>
#include <QObject>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include "private/dplatformwindowinterface_p.h"

DGUI_BEGIN_NAMESPACE
class PersonalizationWindowContext;
class DTreeLandPlatformWindowHelper : public QObject {
    Q_OBJECT
public:
    static DTreeLandPlatformWindowHelper *get(QWindow *window);
    ~DTreeLandPlatformWindowHelper() override;

    enum Feature {
        NoTitlebar  = 1 << 0,
        Radius      = 1 << 1,
        Blur        = 1 << 2,
        Border      = 1 << 3,
        Shadow      = 1 << 4,
    };
    Q_DECLARE_FLAGS(Features, Feature)

    QWindow *window() const { return qobject_cast<QWindow *>(parent()); }
    PersonalizationWindowContext *windowContext() const;

    void setEnabledNoTitlebar(bool enable);
    void setWindowRadius(int windowRadius);
    void setBorderWidth(int borderWidth);
    void setBorderColor(const QColor &borderColor);
    void setShadowRadius(int shadowRadius);
    void setShadowOffset(const QPoint &shadowOffset);
    void setShadowColor(const QColor &shadowColor);
    void setEnableBlurWindow(bool enableBlurWindow);
    void setPlatformHandle(DPlatformHandle *handle);

    bool isEnabledNoTitlebar() const { return m_noTitlebar; }
    int windowRadius() const { return m_radius; }
    int borderWidth() const { return m_borderWidth; }
    QColor borderColor() const { return m_borderColor; }
    int shadowRadius() const { return m_shadowRadius; }
    QPoint shadowOffset() const { return m_shadowOffset; }
    QColor shadowColor() const { return m_shadowColor; }
    bool enableBlurWindow() const { return m_blur; }

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
        m_initialized |= flag;
        m_dirty |= flag;
        scheduleApply();
    }

private:
    PersonalizationWindowContext *m_windowContext = nullptr;
    static QMap<QWindow *, DTreeLandPlatformWindowHelper*> windowMap;
    DPlatformHandle *m_platformHandle = nullptr;

    bool m_noTitlebar = false;
    bool m_blur = false;
    int m_radius = 0;
    int m_borderWidth = 0;
    QColor m_borderColor;
    int m_shadowRadius = 0;
    QPoint m_shadowOffset;
    QColor m_shadowColor;
    Features m_initialized;
    Features m_dirty;
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
};

DGUI_END_NAMESPACE

#endif // DTREELANDPLATFORMWINDOWINTERFACE_H

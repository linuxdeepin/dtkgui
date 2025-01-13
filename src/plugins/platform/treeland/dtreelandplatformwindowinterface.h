// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTREELANDPLATFORMWINDOWINTERFACE_H
#define DTREELANDPLATFORMWINDOWINTERFACE_H

#include "dtkgui_global.h"
#include "dtreelandplatforminterface.h"
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

    QWindow *window() const { return qobject_cast<QWindow *>(parent()); }
    PersonalizationWindowContext *windowContext() const;

    void setEnabledNoTitlebar(bool enable);
    void setWindowRadius(int windowRadius);
    void setEnableBlurWindow(bool enableBlurWindow);

private slots:
    void onActiveChanged();
    void onSurfaceCreated();
    void onSurfaceDestroyed();
private:
    explicit DTreeLandPlatformWindowHelper(QWindow *window);
    bool eventFilter(QObject *watched, QEvent *event) override;
    void initWaylandWindow();

    void doSetEnabledNoTitlebar();
    void doSetWindowRadius();
    void doSetEnabledBlurWindow();
private:
    PersonalizationWindowContext *m_windowContext = nullptr;
    static QMap<QWindow *, DTreeLandPlatformWindowHelper*> windowMap;

    bool m_isNoTitlebar = false;
    bool m_isWindowBlur = false;
    int m_radius = 0;
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

    bool enableBlurWindow() const override;
    void setEnableBlurWindow(bool enableBlurWindow) override;

private:
    bool m_isNoTitlebar = false;
    bool m_isWindowBlur = false;
    int m_radius = 0;
};

DGUI_END_NAMESPACE
#endif // DTREELANDPLATFORMWINDOWINTERFACE_H

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
class DTreeLandPlatformWindowInterface : public QObject, public DPlatformWindowInterface
{
    Q_OBJECT
public:
    explicit DTreeLandPlatformWindowInterface(QWindow *window, DPlatformHandle *platformHandle, QObject *parent = nullptr);
    ~DTreeLandPlatformWindowInterface() override;

    void initWaylandWindow();

    void setEnabled(bool enabled) override;
    bool isEnabled() const override;

    bool isEnabledNoTitlebar() const override;
    bool setEnabledNoTitlebar(bool enable) override;

    int windowRadius() const override;
    void setWindowRadius(int windowRadius) override;

    bool enableBlurWindow() const override;
    void setEnableBlurWindow(bool enableBlurWindow) override;

public slots:
    void onSurfaceCreated();
    void onSurfaceDestroyed();

private:
    PersonalizationWindowContext *getWindowContext();
    void handlePendingTasks();
    void doSetEnabledNoTitlebar();
    void doSetWindowRadius();
    void doSetEnabledBlurWindow();

    QQueue<std::function<void()>> m_pendingTasks;
    PersonalizationManager *m_manager = nullptr;
    PersonalizationWindowContext *m_windowContext = nullptr;
    bool m_isNoTitlebar = false;
    bool m_isWindowBlur = false;
    int m_radius = 0;
};

DGUI_END_NAMESPACE
#endif // DTREELANDPLATFORMWINDOWINTERFACE_H

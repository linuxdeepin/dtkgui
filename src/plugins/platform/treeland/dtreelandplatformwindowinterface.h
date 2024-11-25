// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTREELANDPLATFORMWINDOWINTERFACE_H
#define DTREELANDPLATFORMWINDOWINTERFACE_H

#include "dtkgui_global.h"
#include "dtreelandplatforminterface.h"
#include <QObject>

DGUI_USE_NAMESPACE
class DTreeLandPlatformWindowInterface : public QObject
{
    Q_OBJECT
public:
    explicit DTreeLandPlatformWindowInterface(QObject *parent = nullptr, QWindow *window = nullptr);
    ~DTreeLandPlatformWindowInterface();
    bool setEnabledNoTitlebar(bool enable);
    void setEnableBlurWindow(bool enable);
    void doSetEnabledNoTitlebar();
    [[nodiscard]]QWindow *getWindow() const { return m_window; }

private:
    PersonalizationWindowContext *getWindowContext();
    void handlePendingTasks();

private:
    QWindow *m_window = nullptr;
    QQueue<std::function<void()>> m_pendingTasks;
    PersonalizationManager *m_manager = nullptr;
    PersonalizationWindowContext *m_windowContext = nullptr;
    bool m_isNoTitlebar = true;
};

#endif // DTREELANDPLATFORMWINDOWINTERFACE_H

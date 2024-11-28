// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTREELANDPLATFORMWINDOWINTERFACE_H
#define DTREELANDPLATFORMWINDOWINTERFACE_H

#include "dtkgui_global.h"
#include "private/dplatformwindowinterface_p.h"

DGUI_BEGIN_NAMESPACE

class PersonalizationWindowContext;
class DTreeLandPlatformWindowInterfacePrivate;
class DTreeLandPlatformWindowInterface : public DPlatformWindowInterface
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DTreeLandPlatformWindowInterface)
public:
    explicit DTreeLandPlatformWindowInterface(QWindow *window, DPlatformHandle *platformHandle, QObject *parent = nullptr);
    ~DTreeLandPlatformWindowInterface();

    bool setEnabledNoTitlebar(bool enable) override;
    void setEnableBlurWindow(bool enable) override;
    
    void doSetEnabledNoTitlebar();

private:
    PersonalizationWindowContext *getWindowContext();
    void handlePendingTasks();
};

DGUI_END_NAMESPACE
#endif // DTREELANDPLATFORMWINDOWINTERFACE_H

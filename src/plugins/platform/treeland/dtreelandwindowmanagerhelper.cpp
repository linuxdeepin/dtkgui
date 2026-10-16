// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtreelandwindowmanagerhelper.h"
#include "treelandbackgroundeffectclientextension.h"
#include "treelanddecorationclientextension.h"

DGUI_BEGIN_NAMESPACE

TreelandWindowManagerHelper::TreelandWindowManagerHelper(QObject *parent)
    : DWindowManagerHelper(parent)
{
    connect(BackgroundEffectManager::instance(), &BackgroundEffectManager::blurCapabilityChanged, this, [this](){
        Q_EMIT hasBlurWindowChanged();
    });
    connect(TreelandDecorationManager::instance(), &TreelandDecorationManager::activeChanged, this, [this](){
        Q_EMIT hasNoTitlebarChanged();
    });
}

bool TreelandWindowManagerHelper::hasBlurWindow() const
{
    return BackgroundEffectManager::instance()->isSupported()
        && BackgroundEffectManager::instance()->hasBlurCapability();
}

bool TreelandWindowManagerHelper::hasComposite() const
{
    return true;
}

bool TreelandWindowManagerHelper::hasNoTitlebar() const
{
    return TreelandDecorationManager::instance()->isSupported();
}

DGUI_END_NAMESPACE

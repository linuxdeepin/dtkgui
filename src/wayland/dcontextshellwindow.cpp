// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "dcontextshellwindow.h"

#include "qwaylandpersonalizationshellintegration_p.h"

#include <QtWaylandClient/private/qwaylandwindow_p.h>

class DContextShellWindowPrivate
{
public:
    explicit DContextShellWindowPrivate(QWindow *window)
        : parentWindow(window)
    {
    }

    QWindow *parentWindow = nullptr;
    int noTitlebar = -1;
};

int DContextShellWindow::noTitlebar()
{
    return d->noTitlebar;
}

void DContextShellWindow::setNoTitlebar(const int value)
{
    if (value == d->noTitlebar) {
        return;
    }
    d->noTitlebar = value;
}

static QMap<QWindow *, DContextShellWindow *> s_map;

DContextShellWindow::~DContextShellWindow()
{
    s_map.remove(d->parentWindow);
}

DContextShellWindow::DContextShellWindow(QWindow *window)
    : QObject(window)
    , d(new DContextShellWindowPrivate(window))
{
    s_map.insert(window, this);
    window->create();
    auto waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(window->handle());
    if (waylandWindow) {
        static QWaylandPersonalizationShellIntegration *shellIntegration = nullptr;
        if (!shellIntegration) {
            shellIntegration = new QWaylandPersonalizationShellIntegration();
            if (!shellIntegration->initialize(waylandWindow->display())) {
                delete shellIntegration;
                shellIntegration = nullptr;
                qWarning() << "failed to init dlayershell intergration";
                return;
            }
        }
        waylandWindow->setShellIntegration(shellIntegration);
    } else {
        qWarning() << "not a wayland window, will not create zwlr_layer_surface";
    }
}

DContextShellWindow *DContextShellWindow::get(QWindow *window)
{
    auto dlayerShellWindow = s_map.value(window);
    if (dlayerShellWindow) {
        return dlayerShellWindow;
    }
    return new DContextShellWindow(window);
}

DContextShellWindow *DContextShellWindow::qmlAttachedProperties(QObject *object)
{
    auto window = qobject_cast<QWindow *>(object);
    if (window)
        return get(window);
    qWarning() << "not a qwindow unable to create DContextShellWindow";
    return nullptr;
}

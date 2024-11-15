// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QtWaylandClient/QWaylandClientExtension>

#include "qwayland-treeland-personalization-manager-v1.h"

#include <dtkgui_global.h>
#include <qwaylandclientextension.h>
#include <private/qwaylanddisplay_p.h>

DGUI_BEGIN_NAMESPACE
class PersonalizationWindowContext;
class PersonalizationAppearanceContext;


class PersonalizationManager: public QWaylandClientExtensionTemplate<PersonalizationManager>,
                              public QtWayland::treeland_personalization_manager_v1
{
    Q_OBJECT
public:
    static PersonalizationManager *instance();
    ~PersonalizationManager();

    void setEnableTitleBar(QWindow *window, bool enable);
    bool isSupported() const;

protected:
    explicit PersonalizationManager();

private:
    void addListener();
    void removeListener();
    PersonalizationWindowContext *getWindowContext(QWindow *window);

    static void handleListenerGlobal(void *data, wl_registry *registry, uint32_t id, const QString &interface, uint32_t version);

private:
    QHash<QWindow *, PersonalizationWindowContext *> m_windowContexts;
    QtWaylandClient::QWaylandDisplay *m_waylandDisplay = nullptr;
    bool m_isSupported;
};

class PersonalizationWindowContext : public QWaylandClientExtensionTemplate<PersonalizationWindowContext>,
                                     public QtWayland::treeland_personalization_window_context_v1
{
    Q_OBJECT
public:
    explicit PersonalizationWindowContext(struct ::treeland_personalization_window_context_v1 *context);

    void setEnableTitleBar(bool enable);
};
DGUI_END_NAMESPACE

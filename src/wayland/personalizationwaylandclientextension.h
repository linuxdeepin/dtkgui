// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtWaylandClient/QWaylandClientExtension>
#include <QWindow>

#include "qwayland-treeland-personalization-manager-v1.h"

#include <dtkgui_global.h>
#include <qwaylandclientextension.h>
#include <qwindow.h>

DGUI_BEGIN_NAMESPACE
class PersonalizationWindowContext;
class PersonalizationManager: public QWaylandClientExtensionTemplate<PersonalizationManager>,
                              public QtWayland::treeland_personalization_manager_v1
{
    Q_OBJECT
public:
    static PersonalizationManager *instance();

    bool noTitlebar();
    void setNoTitlebar(bool enable, QWindow *window);

protected:
    explicit PersonalizationManager();

private:
    PersonalizationWindowContext *getWindowContext(QWindow *window);

private:
    QScopedPointer<PersonalizationWindowContext> m_windowContext;
};

class PersonalizationWindowContext : public QWaylandClientExtensionTemplate<PersonalizationWindowContext>,
                                     public QtWayland::treeland_personalization_window_context_v1
{
    Q_OBJECT
public:
    explicit PersonalizationWindowContext(struct ::treeland_personalization_window_context_v1 *context);

    bool noTitlebar();
    void setNoTitlebar(bool enable);

private:
    bool m_noTitlebar;
};


class PersonalizationAppearanceContext : public QWaylandClientExtensionTemplate<PersonalizationAppearanceContext>,
                                         public QtWayland::treeland_personalization_appearance_context_v1
{
    Q_OBJECT
public:
    explicit PersonalizationAppearanceContext(struct ::treeland_personalization_appearance_context_v1 *context);

};

class PersonalizationWallpaperContext : public QWaylandClientExtensionTemplate<PersonalizationWallpaperContext>,
                                        public QtWayland::treeland_personalization_wallpaper_context_v1
{
    Q_OBJECT
public:
    explicit PersonalizationWallpaperContext(struct ::treeland_personalization_wallpaper_context_v1 *context);


};

class PersonalizationCursorContext : public QWaylandClientExtensionTemplate<PersonalizationCursorContext>,
                                     public QtWayland::treeland_personalization_cursor_context_v1
{
    Q_OBJECT
public:
    explicit PersonalizationCursorContext(struct ::treeland_personalization_cursor_context_v1 *context);

};

DGUI_END_NAMESPACE

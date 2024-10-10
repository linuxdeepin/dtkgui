// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtWaylandClient/QWaylandClientExtension>
#include <QWindow>

#include "qwayland-treeland-personalization-manager-v1.h"

#include <dtkgui_global.h>
#include <qhash.h>
#include <qscopedpointer.h>
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

    bool isEnabledNoTitlebar(QWindow *window);
    bool setEnabledNoTitlebar(QWindow *window, bool enable);

    void setWindowRadius(QWindow *window, int radius);

    int windowRadius() const;
    void setWindowRadius(int radius);

    QString fontName() const;
    void setFontName(const QString& fontName);

    QString monoFontName() const;
    void setMonoFontName(const QString& monoFontName);

    QString cursorThemeName() const;
    void setCursorThemeName(const QString& cursorTheme);

    QString iconThemeName() const;
    void setIconThemeName(const QString& iconTheme);

protected:
    explicit PersonalizationManager();

private:
    void addListener();
    void removeListener();
    PersonalizationWindowContext *getWindowContext(QWindow *window);

    static void handleListenerGlobal(void *data, wl_registry *registry, uint32_t id, const QString &interface, uint32_t version);

private:
    QScopedPointer<PersonalizationWindowContext> m_windowContext;
    QScopedPointer<PersonalizationAppearanceContext> m_appearanceContext;
    QScopedPointer<QtWaylandClient::QWaylandDisplay> m_waylandDisplay;
    QHash<QWindow *, bool> m_isNoTitlebarMap;
};

class PersonalizationWindowContext : public QWaylandClientExtensionTemplate<PersonalizationWindowContext>,
                                     public QtWayland::treeland_personalization_window_context_v1
{
    Q_OBJECT
public:
    explicit PersonalizationWindowContext(struct ::treeland_personalization_window_context_v1 *context);

    bool noTitlebar() const;
    void setNoTitlebar(bool enable);

    int windowRadius();
    void setWindowRadius(int radius);

private:
    bool m_noTitlebar;
    int m_windowRadius;
};

class PersonalizationAppearanceContext : public QWaylandClientExtensionTemplate<PersonalizationAppearanceContext>,
                                         public QtWayland::treeland_personalization_appearance_context_v1
{
    Q_OBJECT
    Q_PROPERTY(int windowRadius READ windowRadius WRITE setWindowRadius)
    Q_PROPERTY(QString fontName READ fontName WRITE setFontName)
    Q_PROPERTY(QString monoFontName READ monoFontName WRITE setMonoFontName)
    Q_PROPERTY(QString cursorTheme READ cursorTheme WRITE setCursorTheme)
    Q_PROPERTY(QString iconTheme READ iconTheme WRITE setIconTheme)

public:
    explicit PersonalizationAppearanceContext(struct ::treeland_personalization_appearance_context_v1 *context);

    int windowRadius() const;
    void setWindowRadius(int radius);

    QString fontName() const;
    void setFontName(const QString& fontName);

    QString monoFontName() const;
    void setMonoFontName(const QString& monoFontName);

    QString cursorTheme() const;
    void setCursorTheme(const QString& cursorTheme);

    QString iconTheme() const;
    void setIconTheme(const QString& iconTheme);

protected:
    virtual void treeland_personalization_appearance_context_v1_round_corner_radius(int32_t radius);
    virtual void treeland_personalization_appearance_context_v1_font(const QString &font_name);
    virtual void treeland_personalization_appearance_context_v1_monospace_font(const QString &font_name);
    virtual void treeland_personalization_appearance_context_v1_cursor_theme(const QString &theme_name);
    virtual void treeland_personalization_appearance_context_v1_icon_theme(const QString &theme_name);

private:
    int m_windowRadius;
    QString m_fontName;
    QString m_monoFontName;
    QString m_cursorTheme;
    QString m_iconTheme;
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

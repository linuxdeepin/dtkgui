/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DWINDOWMANAGERHELPER_H
#define DWINDOWMANAGERHELPER_H

#include <dtkgui_global.h>
#include <DObject>

#include <QWindow>

DGUI_BEGIN_NAMESPACE

class DForeignWindow;
class DWindowManagerHelperPrivate;
class DWindowManagerHelper : public QObject, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT

    Q_PROPERTY(bool hasBlurWindow READ hasBlurWindow NOTIFY hasBlurWindowChanged)
    Q_PROPERTY(bool hasComposite READ hasComposite NOTIFY hasCompositeChanged)
    Q_PROPERTY(bool hasNoTitlebar READ hasNoTitlebar NOTIFY hasNoTitlebarChanged)
    Q_PROPERTY(bool hasWallpaperEffect READ hasWallpaperEffect NOTIFY hasWallpaperEffectChanged)

public:
    enum MotifFunction {
        FUNC_RESIZE   = (1L << 1),
        FUNC_MOVE     = (1L << 2),
        FUNC_MINIMIZE = (1L << 3),
        FUNC_MAXIMIZE = (1L << 4),
        FUNC_CLOSE    = (1L << 5),
        FUNC_ALL      = FUNC_RESIZE | FUNC_MOVE | FUNC_MINIMIZE | FUNC_MAXIMIZE | FUNC_CLOSE
    };
    Q_DECLARE_FLAGS(MotifFunctions, MotifFunction)

    enum MotifDecoration {
        DECOR_BORDER   = (1L << 1),
        DECOR_RESIZEH  = (1L << 2),
        DECOR_TITLE    = (1L << 3),
        DECOR_MENU     = (1L << 4),
        DECOR_MINIMIZE = (1L << 5),
        DECOR_MAXIMIZE = (1L << 6),
        DECOR_ALL      = DECOR_BORDER | DECOR_RESIZEH | DECOR_TITLE | DECOR_MENU | DECOR_MINIMIZE | DECOR_MAXIMIZE
    };
    Q_DECLARE_FLAGS(MotifDecorations, MotifDecoration)

    enum WMName {
        OtherWM,
        DeepinWM,
        KWinWM
    };
    Q_ENUM(WMName)

    enum WmWindowType {
        UnknowWindowType = 0x000000,
        NormalType       = 0x000001,
        DesktopType      = 0x000002,
        DockType         = 0x000004,
        ToolbarType      = 0x000008,
        MenuType         = 0x000010,
        UtilityType      = 0x000020,
        SplashType       = 0x000040,
        DialogType       = 0x000080,
        DropDownMenuType = 0x000100,
        PopupMenuType    = 0x000200,
        TooltipType      = 0x000400,
        NotificationType = 0x000800,
        ComboType        = 0x001000,
        DndType          = 0x002000,
        KdeOverrideType  = 0x004000
    };
    Q_ENUM(WmWindowType)
    Q_DECLARE_FLAGS(WmWindowTypes, WmWindowType)
    Q_FLAG(WmWindowTypes)

    ~DWindowManagerHelper();

    static DWindowManagerHelper *instance();

    static void setMotifFunctions(const QWindow *window, MotifFunctions hints);
    static MotifFunctions setMotifFunctions(const QWindow *window, MotifFunctions hints, bool on);
    static MotifFunctions getMotifFunctions(const QWindow *window);
    static void setMotifDecorations(const QWindow *window, MotifDecorations hints);
    static MotifDecorations setMotifDecorations(const QWindow *window, MotifDecorations hints, bool on);
    static MotifDecorations getMotifDecorations(const QWindow *window);
    static void setWmWindowTypes(QWindow *window, WmWindowTypes types);
    static void setWmClassName(const QByteArray &name);

    static void popupSystemWindowMenu(const QWindow *window);

    bool hasBlurWindow() const;
    bool hasComposite() const;
    bool hasNoTitlebar() const;
    bool hasWallpaperEffect() const;
    QString windowManagerNameString() const;
    WMName windowManagerName() const;

    QVector<quint32> allWindowIdList() const;
    QVector<quint32> currentWorkspaceWindowIdList() const;
    QList<DForeignWindow*> currentWorkspaceWindows() const;
    quint32 windowFromPoint(const QPoint &p);

Q_SIGNALS:
    void windowManagerChanged();
    void hasBlurWindowChanged();
    void hasCompositeChanged();
    void hasNoTitlebarChanged();
    void hasWallpaperEffectChanged();
    void windowListChanged();
    void windowMotifWMHintsChanged(quint32 winId);

protected:
    explicit DWindowManagerHelper(QObject *parent = 0);

private:
    D_DECLARE_PRIVATE(DWindowManagerHelper)
};

DGUI_END_NAMESPACE

Q_DECLARE_OPERATORS_FOR_FLAGS(DTK_GUI_NAMESPACE::DWindowManagerHelper::MotifFunctions)
Q_DECLARE_OPERATORS_FOR_FLAGS(DTK_GUI_NAMESPACE::DWindowManagerHelper::MotifDecorations)
Q_DECLARE_OPERATORS_FOR_FLAGS(DTK_GUI_NAMESPACE::DWindowManagerHelper::WmWindowTypes)

#endif // DWINDOWMANAGERHELPER_H

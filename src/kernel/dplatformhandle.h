// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLATFORMHANDLE_H
#define DPLATFORMHANDLE_H

#include <dtkgui_global.h>

#include <QObject>
#include <QPainterPath>
#include <QColor>
#include <QRegion>

#include <functional>

QT_BEGIN_NAMESPACE
class QWindow;
QT_END_NAMESPACE

DGUI_BEGIN_NAMESPACE

class DPlatformHandle : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int windowRadius READ windowRadius WRITE setWindowRadius NOTIFY windowRadiusChanged)
    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(int shadowRadius READ shadowRadius WRITE setShadowRadius NOTIFY shadowRadiusChanged)
    Q_PROPERTY(QPoint shadowOffset READ shadowOffset WRITE setShadowOffset NOTIFY shadowOffsetChanged)
    Q_PROPERTY(QColor shadowColor READ shadowColor WRITE setShadowColor NOTIFY shadowColorChanged)
    Q_PROPERTY(QPainterPath clipPath READ clipPath WRITE setClipPath NOTIFY clipPathChanged)
    Q_PROPERTY(QRegion frameMask READ frameMask WRITE setFrameMask NOTIFY frameMaskChanged)
    Q_PROPERTY(QMargins frameMargins READ frameMargins NOTIFY frameMarginsChanged)
    Q_PROPERTY(bool translucentBackground READ translucentBackground WRITE setTranslucentBackground NOTIFY translucentBackgroundChanged)
    Q_PROPERTY(bool enableSystemResize READ enableSystemResize WRITE setEnableSystemResize NOTIFY enableSystemResizeChanged)
    Q_PROPERTY(bool enableSystemMove READ enableSystemMove WRITE setEnableSystemMove NOTIFY enableSystemMoveChanged)
    Q_PROPERTY(bool enableBlurWindow READ enableBlurWindow WRITE setEnableBlurWindow NOTIFY enableBlurWindowChanged)
    Q_PROPERTY(bool autoInputMaskByClipPath READ autoInputMaskByClipPath WRITE setAutoInputMaskByClipPath NOTIFY autoInputMaskByClipPathChanged)
    Q_PROPERTY(WId realWindowId READ realWindowId CONSTANT)

public:
    explicit DPlatformHandle(QWindow *window, QObject *parent = 0);

    static QString pluginVersion();
    static bool isDXcbPlatform();

    static void enableDXcbForWindow(QWindow *window);
    static void enableDXcbForWindow(QWindow *window, bool redirectContent);
    static bool isEnabledDXcb(const QWindow *window);

    static bool setEnabledNoTitlebarForWindow(QWindow *window, bool enable);
    static bool isEnabledNoTitlebar(const QWindow *window);

    struct WMBlurArea {
        qint32 x = 0;
        qint32 y = 0;
        qint32 width = 0;
        qint32 height = 0;
        qint32 xRadius = 0;
        qint32 yRaduis = 0;
    };

    enum WallpaperScaleMode {
        FollowScreen = 0x00000000,
        FollowWindow = 0x00010000
    };

    enum WallpaperFillMode {
        PreserveAspectCrop = 0x00000000,
        PreserveAspectFit = 0x00000001
    };

    static bool setWindowBlurAreaByWM(QWindow *window, const QVector<WMBlurArea> &area);
    static bool setWindowBlurAreaByWM(QWindow *window, const QList<QPainterPath> &paths);
    static bool setWindowWallpaperParaByWM(QWindow *window, const QRect &area, WallpaperScaleMode sMode, WallpaperFillMode fMode);
    static bool connectWindowManagerChangedSignal(QObject *object, std::function<void ()> slot);
    static bool connectHasBlurWindowChanged(QObject *object, std::function<void ()> slot);

    bool setWindowBlurAreaByWM(const QVector<WMBlurArea> &area);
    bool setWindowBlurAreaByWM(const QList<QPainterPath> &paths);

    static void setDisableWindowOverrideCursor(QWindow *window, bool disable);

    int windowRadius() const;

    int borderWidth() const;
    QColor borderColor() const;

    int shadowRadius() const;
    QPoint shadowOffset() const;
    QColor shadowColor() const;

    QPainterPath clipPath() const;
    QRegion frameMask() const;
    QMargins frameMargins() const;

    bool translucentBackground() const;
    bool enableSystemResize() const;
    bool enableSystemMove() const;
    bool enableBlurWindow() const;
    bool autoInputMaskByClipPath() const;

    WId realWindowId() const;
    static WId windowLeader();

public Q_SLOTS:
    void setWindowRadius(int windowRadius);

    void setBorderWidth(int borderWidth);
    void setBorderColor(const QColor &borderColor);

    void setShadowRadius(int shadowRadius);
    void setShadowOffset(const QPoint &shadowOffset);
    void setShadowColor(const QColor &shadowColor);

    void setClipPath(const QPainterPath &clipPath);
    void setFrameMask(const QRegion &frameMask);

    void setTranslucentBackground(bool translucentBackground);
    void setEnableSystemResize(bool enableSystemResize);
    void setEnableSystemMove(bool enableSystemMove);
    void setEnableBlurWindow(bool enableBlurWindow);
    void setAutoInputMaskByClipPath(bool autoInputMaskByClipPath);

Q_SIGNALS:
    void frameMarginsChanged();
    void windowRadiusChanged();
    void borderWidthChanged();
    void borderColorChanged();
    void shadowRadiusChanged();
    void shadowOffsetChanged();
    void shadowColorChanged();
    void clipPathChanged();
    void frameMaskChanged();
    void translucentBackgroundChanged();
    void enableSystemResizeChanged();
    void enableSystemMoveChanged();
    void enableBlurWindowChanged();
    void autoInputMaskByClipPathChanged();

protected:
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

private:
    QWindow *m_window;
};

inline DPlatformHandle::WMBlurArea dMakeWMBlurArea(quint32 x, quint32 y, quint32 width, quint32 height, quint32 xr = 0, quint32 yr = 0)
{
    DPlatformHandle::WMBlurArea a;

    a.x = x;
    a.y = y;
    a.width = width;
    a.height = height;
    a.xRadius = xr;
    a.yRaduis = yr;

    return a;
}

DGUI_END_NAMESPACE

QT_BEGIN_NAMESPACE
DGUI_USE_NAMESPACE
QDebug operator<<(QDebug deg, const DPlatformHandle::WMBlurArea &area);
QT_END_NAMESPACE

Q_DECLARE_METATYPE(QPainterPath)
Q_DECLARE_METATYPE(QRegion)
Q_DECLARE_METATYPE(QMargins)

#endif // DPLATFORMHANDLE_H

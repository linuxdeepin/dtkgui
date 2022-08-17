// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DREGIONMONITOR_H
#define DREGIONMONITOR_H

#include <DObject>
#include <dtkgui_global.h>

#include <QObject>

DGUI_BEGIN_NAMESPACE

class DRegionMonitorPrivate;
class DRegionMonitor : public QObject, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DRegionMonitor)
    Q_DISABLE_COPY(DRegionMonitor)
    Q_PROPERTY(CoordinateType coordinateType READ coordinateType WRITE setCoordinateType NOTIFY coordinateTypeChanged)
    Q_PROPERTY(RegisterdFlags registerdFlags READ registerFlags WRITE setRegisterFlags NOTIFY registerdFlagsChanged)

public:
    explicit DRegionMonitor(QObject *parent = nullptr);

    enum RegisterdFlag {
        Motion = 1 << 0,
        Button = 1 << 1,
        Key = 1 << 2
    };
    Q_DECLARE_FLAGS(RegisterdFlags, RegisterdFlag)

    enum WatchedFlags {
        Button_Left     = 1,
        Button_Middle,
        Button_Right,
        Wheel_Up,
        Wheel_Down
    };

    enum CoordinateType {
        ScaleRatio,
        Original
    };
    Q_ENUM(CoordinateType)

    bool registered() const;
    QRegion watchedRegion() const;
    RegisterdFlags registerFlags() const;
    CoordinateType coordinateType() const;

Q_SIGNALS:
    void buttonPress(const QPoint &p, const int flag) const;
    void buttonRelease(const QPoint &p, const int flag) const;
    void cursorMove(const QPoint &p) const;
    void cursorEnter(const QPoint &p) const;
    void cursorLeave(const QPoint &p) const;
    void keyPress(const QString &keyname) const;
    void keyRelease(const QString &keyname) const;
    void registerdFlagsChanged(RegisterdFlags flags) const;
    void coordinateTypeChanged(CoordinateType type) const;

public Q_SLOTS:
    void registerRegion();
    inline void registerRegion(const QRegion &region) { setWatchedRegion(region); registerRegion(); }
    void unregisterRegion();
    void setWatchedRegion(const QRegion &region);
    void setRegisterFlags(RegisterdFlags flags);
    void setCoordinateType(CoordinateType type);

private:
    Q_PRIVATE_SLOT(d_func(), void _q_ButtonPress(const int, const int, const int, const QString&))
    Q_PRIVATE_SLOT(d_func(), void _q_ButtonRelease(const int, const int, const int, const QString&))
    Q_PRIVATE_SLOT(d_func(), void _q_CursorMove(const int, const int, const QString &))
    Q_PRIVATE_SLOT(d_func(), void _q_CursorEnter(const int, const int, const QString &))
    Q_PRIVATE_SLOT(d_func(), void _q_CursorLeave(const int, const int, const QString &))
    Q_PRIVATE_SLOT(d_func(), void _q_KeyPress(const QString &, const int, const int, const QString &))
    Q_PRIVATE_SLOT(d_func(), void _q_KeyRelease(const QString &, const int, const int, const QString &))
};

Q_DECLARE_OPERATORS_FOR_FLAGS (DRegionMonitor::RegisterdFlags);

DGUI_END_NAMESPACE

#endif // DREGIONMONITOR_H

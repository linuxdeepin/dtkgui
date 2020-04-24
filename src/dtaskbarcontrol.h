/*
 * Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     sunkang <sunkang@uniontech.com>
 *
 * Maintainer: sunkang <sunkang@uniontech.com>
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

#ifndef DTASKBARCONTROL_H
#define DTASKBARCONTROL_H

#include <dtkgui_global.h>

#include <QObject>
#include <QColor>

#include <DObject>

DGUI_BEGIN_NAMESPACE
class DTaskbarControlPrivate;

class DTaskbarControl : public QObject, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT

public:
    DTaskbarControl(QObject *parent = nullptr);
    ~DTaskbarControl();

    void setProgress(bool progressVisible, double progress);
    void setCounter(bool counterVisible, int counter);
    int counter() const;
    void setCounterVisible(bool counterVisible);
    bool counterVisible() const;
    void setUrgency(bool val);

Q_SIGNALS:
    void counterChanged(int counter);
    void counterVisibleChanged(bool visible);
    void progressChanged(double progress);
    void progressVisibleChanged(bool visible);

protected:
    virtual void sendMessage(const QVariantMap &params);

private:
    D_DECLARE_PRIVATE(DTaskbarControl)
};



DGUI_END_NAMESPACE

#endif // DTASKBARCONTROL_H

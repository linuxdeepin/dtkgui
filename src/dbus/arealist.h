// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef AREALIST_H
#define AREALIST_H

#include <QDBusMetaType>
#include <QRect>
#include <QList>

typedef QList<QRect> AreaList;

void registerAreaListMetaType();

#endif // AREALIST_H

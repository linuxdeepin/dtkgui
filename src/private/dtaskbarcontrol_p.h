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

#ifndef DTASKBARCONTROL_P_H
#define DTASKBARCONTROL_P_H

#include <dtkgui_global.h>
#include <DObjectPrivate>
#include <DObject>

#include "dtaskbarcontrol.h"

DCORE_USE_NAMESPACE
DGUI_BEGIN_NAMESPACE

class DTaskbarControlPrivate : public DObjectPrivate
{
    D_DECLARE_PUBLIC(DTaskbarControl)

public:
    DTaskbarControlPrivate(DTaskbarControl *q);
    ~DTaskbarControlPrivate();

    int m_counter;
    bool m_counterVisible;
    double m_progress;
    bool m_progressVisible;
};

DGUI_END_NAMESPACE

#endif // DTASKBARCONTROL_P_H

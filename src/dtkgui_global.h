/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
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
#ifndef DTKGUI_GLOBAL_H
#define DTKGUI_GLOBAL_H

#pragma once

#include <dtkcore_global.h>

#define DGUI_NAMESPACE Gui
#define DTK_GUI_NAMESPACE DTK_NAMESPACE::Gui

#define DGUI_BEGIN_NAMESPACE namespace DTK_NAMESPACE { namespace DGUI_NAMESPACE {
#define DGUI_END_NAMESPACE }}
#define DGUI_USE_NAMESPACE using namespace DTK_GUI_NAMESPACE;

#endif // DTKGUI_GLOBAL_H

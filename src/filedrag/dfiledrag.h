/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Chris Xiong <chirs241097@gmail.com>
 *
 * Maintainer: Chris Xiong <chirs241097@gmail.com>
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

#ifndef DFILEDRAG_H
#define DFILEDRAG_H

#include <dtkgui_global.h>

#include <DObject>

#include <QDrag>
#include <QUrl>

DGUI_BEGIN_NAMESPACE

class DFileDragServer;
class DFileDragPrivate;

class DFileDrag : public QDrag, public DCORE_NAMESPACE::DObject
{
    Q_OBJECT
public:
    explicit DFileDrag(QObject *source, DFileDragServer *server);

    QUrl targetUrl();
    void setMimeData(QMimeData *data);

Q_SIGNALS:
    void targetUrlChanged(QUrl url);

private:
    D_DECLARE_PRIVATE(DFileDrag)
};

DGUI_END_NAMESPACE

#endif // DFILEDRAG_H

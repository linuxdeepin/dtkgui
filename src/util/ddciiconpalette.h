/*
 * Copyright (C) 2022 UnionTech Technology Co., Ltd.
 *
 * Author:     Chen Bin <chenbin@uniontech.com>
 *
 * Maintainer: Chen Bin <chenbin@uniontech.com>
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DDCIICONPALETTE_H
#define DDCIICONPALETTE_H

#include <dtkgui_global.h>
#include <qobjectdefs.h>

#include <QColor>
#include <QVector>
#include <QUrl>

DGUI_BEGIN_NAMESPACE

class DDciIconPalette
{
    Q_GADGET
    Q_PROPERTY(QColor foreground READ foreground WRITE setForeground FINAL)
    Q_PROPERTY(QColor background READ background WRITE setBackground FINAL)
    Q_PROPERTY(QColor highlight READ highlight WRITE setHighlight FINAL)
    Q_PROPERTY(QColor highlightForeground READ highlightForeground WRITE setHighlightForeground FINAL)

public:
    enum PaletteRole {
        NoPalette = -1,
        Foreground = 0,
        Background = 1,
        HighlightForeground = 2,
        Highlight = 3,
        PaletteCount
    };

    DDciIconPalette(QColor foreground = QColor::Invalid, QColor background = QColor::Invalid,
                    QColor highlight = QColor::Invalid, QColor highlightForeground = QColor::Invalid);
    bool operator==(const DDciIconPalette &other) const;
    bool operator!=(const DDciIconPalette &other) const;

    QColor foreground() const;
    void setForeground(const QColor &foreground);

    QColor background() const;
    void setBackground(const QColor &background);

    QColor highlightForeground() const;
    void setHighlightForeground(const QColor &highlightForeground);

    QColor highlight() const;
    void setHighlight(const QColor &highlight);

    static QUrl convertToUrl(const DDciIconPalette &palette);
    static DDciIconPalette convertFromUrl(const QUrl &url);
private:
    QVector<QColor> colors;
};

DGUI_END_NAMESPACE

QT_BEGIN_NAMESPACE
#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug, const DTK_GUI_NAMESPACE::DDciIconPalette &);
#endif
QT_END_NAMESPACE
#endif // DDCIICONPALETTE_H

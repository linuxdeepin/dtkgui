/*
 * Copyright (C) 2021 UnionTech Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@uniontech.com>
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
#pragma once

#include <dtkgui_global.h>
#include <functional>

#include <QBrush>

DCORE_BEGIN_NAMESPACE
class DDciFile;
DCORE_END_NAMESPACE

DGUI_BEGIN_NAMESPACE

class DDciIconPrivate;
class DDciIcon
{
public:
    enum Type {
        TextType = 0,
        ActionType = 1,
        IconType = 2,

        TypeCount,
        CurrentType = 1 << 24,
    };
    enum Mode {
        Normal = 0,
        Disabled = 1,
        Hover = 2,
        Pressed = 3
    };
    enum Theme {
        Light = 0,
        Dark = 1
    };
    enum MatchOption {
        NoMatchOption = 0,
        ModeSensitive = 1 << 0,
        FormatSensitive = 1 << 1,
        AllMatchOptions = ModeSensitive | FormatSensitive
    };
    Q_DECLARE_FLAGS(MatchOptions, MatchOption)

    DDciIcon(Type ct = DDciIcon::TextType);
    explicit DDciIcon(const DCORE_NAMESPACE::DDciFile *dciFile, DDciIcon::Type ct = DDciIcon::TextType);
    explicit DDciIcon(const QString &fileName, DDciIcon::Type ct = DDciIcon::TextType);
    explicit DDciIcon(const QByteArray &data, DDciIcon::Type ct = DDciIcon::TextType);
    DDciIcon(const DDciIcon &other);
    DDciIcon &operator=(const DDciIcon &other) noexcept;
    ~DDciIcon();
    DDciIcon(DDciIcon && other) noexcept;
    DDciIcon &operator=(DDciIcon &&other) noexcept;
    void swap(DDciIcon &other) noexcept { d.swap(other.d); }

    void setCurrentType(Type type);
    Type currentType() const;

    bool isNull(DDciIcon::Type type = CurrentType) const;

    struct Icon : public QSharedData {
        int iconSize;
        Type type;
        Mode mode;
        Theme theme;
        QByteArray format;

        struct Data {
            int imagePixelRatio;
            QByteArray foreground;
            QByteArray background;
        };
        QVector<Data> datas;
    };
    using IconPointer = QSharedDataPointer<Icon>;

    using IconMatcher = std::function<bool(const IconPointer&)>;
    IconPointer findIcon(IconMatcher matcher) const;
    IconPointer findIcon(int iconSize, Theme theme, Mode mode = Normal, Type type = CurrentType,
                         const QByteArray &requestFormat = QByteArray(),
                         MatchOptions options = NoMatchOption) const;

    static QPixmap generatePixmap(const IconPointer &icon, Mode mode, int iconSize,
                                  qreal devicePixelRatio, const QBrush &foreground = QBrush());
    static void paint(const IconPointer &icon, Mode mode, int iconSize, qreal devicePixelRatio,
                      QPainter *painter, const QRect &rect, Qt::Alignment alignment = Qt::AlignCenter);
    // TODO: Should be compatible with QIcon
private:
    QSharedDataPointer<DDciIconPrivate> d;
    DDciIcon::Type currentIconType;
};

Q_DECLARE_INCOMPATIBLE_FLAGS(DDciIcon::MatchOptions)

DGUI_END_NAMESPACE

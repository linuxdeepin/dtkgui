/*
 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
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

#ifndef DFONTSIZEMANAGER_H
#define DFONTSIZEMANAGER_H
#include <DObject>

#include <dtkgui_global.h>

#include <QFont>
#include <QEvent>

DGUI_BEGIN_NAMESPACE

class DFontManagerPrivate;
class DFontManager : public QObject
    , public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT

    Q_PROPERTY(QFont t1 READ t1 NOTIFY t1Changed)
    Q_PROPERTY(QFont t2 READ t2 NOTIFY t2Changed)
    Q_PROPERTY(QFont t3 READ t3 NOTIFY t3Changed)
    Q_PROPERTY(QFont t4 READ t4 NOTIFY t4Changed)
    Q_PROPERTY(QFont t5 READ t5 NOTIFY t5Changed)
    Q_PROPERTY(QFont t6 READ t6 NOTIFY t6Changed)
    Q_PROPERTY(QFont t7 READ t7 NOTIFY t7Changed)
    Q_PROPERTY(QFont t8 READ t8 NOTIFY t8Changed)
    Q_PROPERTY(QFont t9 READ t9 NOTIFY t9Changed)
    Q_PROPERTY(QFont t10 READ t10 NOTIFY t10Changed)

    Q_PROPERTY(int fontGenericPixelSize READ fontGenericPixelSize WRITE setFontGenericPixelSize NOTIFY fontGenericPixelSizeChanged)

public:
    enum SizeType {
        T1,
        T2,
        T3,
        T4,
        T5,
        T6,
        T7,
        T8,
        T9,
        T10,
        NSizeTypes
    };
    Q_ENUM(SizeType)

    DFontManager(QObject *parent = nullptr);
    ~DFontManager() override;

    static DFontManager *instance();

    Q_INVOKABLE int fontPixelSize(SizeType type) const;
    Q_INVOKABLE void setFontPixelSize(SizeType type, int size);

    void setFontGenericPixelSize(int size);
    int fontGenericPixelSize() const;

    Q_INVOKABLE static int fontPixelSize(const QFont &font);

    Q_INVOKABLE const QFont get(SizeType type, const QFont &base = QFont()) const;
    Q_INVOKABLE const QFont get(SizeType type, int weight, const QFont &base = QFont()) const;

    inline const QFont t1(const QFont &base = QFont()) const
    {
        return get(T1, base);
    }
    inline const QFont t2(const QFont &base = QFont()) const
    {
        return get(T2, base);
    }
    inline const QFont t3(const QFont &base = QFont()) const
    {
        return get(T3, base);
    }
    inline const QFont t4(const QFont &base = QFont()) const
    {
        return get(T4, base);
    }
    inline const QFont t5(const QFont &base = QFont()) const
    {
        return get(T5, base);
    }
    inline const QFont t6(const QFont &base = QFont()) const
    {
        return get(T6, base);
    }
    inline const QFont t7(const QFont &base = QFont()) const
    {
        return get(T7, base);
    }
    inline const QFont t8(const QFont &base = QFont()) const
    {
        return get(T8, base);
    }
    inline const QFont t9(const QFont &base = QFont()) const
    {
        return get(T9, base);
    }
    inline const QFont t10(const QFont &base = QFont()) const
    {
        return get(T10, base);
    }

Q_SIGNALS:
    void t1Changed();
    void t2Changed();
    void t3Changed();
    void t4Changed();
    void t5Changed();
    void t6Changed();
    void t7Changed();
    void t8Changed();
    void t9Changed();
    void t10Changed();

    void fontGenericPixelSizeChanged();

private:
    D_DECLARE_PRIVATE(DFontManager)
};

DGUI_END_NAMESPACE

#endif // DFONTSIZEMANAGER_H

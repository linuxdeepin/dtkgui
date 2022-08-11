// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBUILTINICONENGINE_H
#define DBUILTINICONENGINE_H

#include <dtkgui_global.h>

#include <QIconEngine>
#include <private/qiconloader_p.h>

DGUI_BEGIN_NAMESPACE

// 内置的主题引擎，会从Qt资源文件中查找图标
class DBuiltinIconEnginePrivate;
class Q_DECL_HIDDEN DBuiltinIconEngine : public QIconEngine
{
public:
    DBuiltinIconEngine(const QString &iconName);
    ~DBuiltinIconEngine();
    void paint(QPainter *painter, const QRect &rect,
               QIcon::Mode mode, QIcon::State state);
    QSize actualSize(const QSize &size, QIcon::Mode mode,
                     QIcon::State state);
    QPixmap pixmap(const QSize &size, QIcon::Mode mode,
                   QIcon::State state);

    QString key() const;
    QIconEngine *clone() const;
    bool read(QDataStream &in);
    bool write(QDataStream &out) const;

    QString iconName() const override;

    static QThemeIconInfo loadIcon(const QString &iconName, uint key);

private:
    bool hasIcon() const;
    void ensureLoaded();
    void virtual_hook(int id, void *data) override;

    DBuiltinIconEngine(const DBuiltinIconEngine &other);
    QThemeIconInfo m_info;
    QString m_iconName;
    // 图标的类型(Dark/Light)
    uint m_key:2;
    // 记录是否已经初始化
    bool m_initialized:1;
    // 控制是否跟随系统级别的主题色来改变图标类型
    bool m_followSystemTheme:1;

    friend class QIconLoader;
};

DGUI_END_NAMESPACE

#endif // DBUILTINICONENGINE_H

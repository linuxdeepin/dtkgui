// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWindow>

class DContextShellWindowPrivate;

class DContextShellWindow : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int noTitlebar READ noTitlebar WRITE setNoTitlebar NOTIFY noTitlebarChanged)

public:
    ~DContextShellWindow() override;

    int noTitlebar();
    void setNoTitlebar(const int value);

    static DContextShellWindow *get(QWindow *window);
    static DContextShellWindow *qmlAttachedProperties(QObject *object);

Q_SIGNALS:
    void noTitlebarChanged();

private:
    DContextShellWindow(QWindow *window);
    QScopedPointer<DContextShellWindowPrivate> d;
};

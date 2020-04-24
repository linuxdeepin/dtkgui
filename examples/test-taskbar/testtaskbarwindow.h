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

#ifndef TESTTASKBARWINDOW_H
#define TESTTASKBARWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QUuid>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QColor>
#include <QCheckBox>
#include <QSlider>
#include <QColorDialog>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "dtaskbarcontrol.h"

DGUI_USE_NAMESPACE

class TestTaskbarWindow : public QWidget
{
    Q_OBJECT

public:
    TestTaskbarWindow(QWidget *parent = nullptr);
    ~TestTaskbarWindow();

private:
    DTaskbarControl *m_pTaskbarControl;
    QCheckBox *m_pProgressBox;
    QCheckBox *m_pCounterBox;
    QLineEdit *m_pNumEdit;
    QSlider *m_pProgress;
    QCheckBox *m_pUrgencyBox;
};

#endif // TESTTASKBARWINDOW_H

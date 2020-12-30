/*
 * Copyright (C) 2019~ 2020 Deepin Technology Co., Ltd.
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

#include "testtaskbarwindow.h"

#include <QIntValidator>
#include <QApplication>
#include <QCloseEvent>

TestTaskbarWindow::TestTaskbarWindow(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *pHBoxLayout_1 = new QHBoxLayout;
    m_pProgressBox = new QCheckBox("progress");
    m_pProgress = new QSlider(Qt::Horizontal);
    m_pProgress->setRange(0, 100);
    m_pProgress->setMinimumWidth(300);
    pHBoxLayout_1->addWidget(m_pProgressBox);
    pHBoxLayout_1->addWidget(m_pProgress);

    QHBoxLayout *pHBoxLayout_2 = new QHBoxLayout;
    m_pCounterBox = new QCheckBox("counter");
    m_pNumEdit = new QLineEdit;
    m_pNumEdit->setMinimumWidth(300);
    m_pNumEdit->setValidator(new QIntValidator);
    pHBoxLayout_2->addWidget(m_pCounterBox);
    pHBoxLayout_2->addWidget(m_pNumEdit);

    m_pUrgencyBox= new QCheckBox("test urgency");
    QHBoxLayout *pHBoxLayout_3 = new QHBoxLayout;
    pHBoxLayout_3->addWidget(m_pUrgencyBox);

    QVBoxLayout *pVBoxLayout = new QVBoxLayout;
    setLayout(pVBoxLayout);
    pVBoxLayout->addStretch();
    pVBoxLayout->addLayout(pHBoxLayout_1);
    pVBoxLayout->addSpacing(30);
    pVBoxLayout->addLayout(pHBoxLayout_2);
    pVBoxLayout->addSpacing(50);
    pVBoxLayout->addLayout(pHBoxLayout_3);
    pVBoxLayout->addStretch();

    m_pTaskbarControl = new DTaskbarControl(this);
    m_pTaskbarControl->setProgress(false, 0);
    m_pTaskbarControl->setCounter(false, 0);

    connect(m_pProgressBox, &QCheckBox::stateChanged, this, [this] (int state) {
        m_pTaskbarControl->setProgress(state == Qt::Checked, static_cast<double>(m_pProgress->value()) / 100);
    });

    connect(m_pCounterBox, &QCheckBox::stateChanged, this, [this] (int state) {
        m_pTaskbarControl->setCounterVisible(state == Qt::Checked);
    });

    connect(m_pProgress, &QSlider::valueChanged, this, [this] (int value) {
        m_pTaskbarControl->setProgress(m_pProgressBox->checkState() == Qt::Checked, static_cast<double>(value) / 100);
    });

    connect(m_pNumEdit, &QLineEdit::textChanged, [this] (const QString &str) {
        m_pTaskbarControl->setCounter(m_pCounterBox->checkState() == Qt::Checked, str.toInt());
    });

    connect(m_pUrgencyBox, &QCheckBox::stateChanged, this, [this] (int state) {
        m_pTaskbarControl->setUrgency(state == Qt::Checked);
    });
}

TestTaskbarWindow::~TestTaskbarWindow()
{

}

void TestTaskbarWindow::closeEvent(QCloseEvent *event)
{
    Q_EMIT closeWindow();
    event->accept();
}

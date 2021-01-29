#include <gtest/gtest.h>
#include "dforeignwindow.h"

#include <dwindowmanagerhelper.h>
#include <QDebug>

DGUI_BEGIN_NAMESPACE

class TDForeignWindow : public testing::Test
{
protected:
    virtual void SetUp()
    {
        const QVector<quint32> &currentIdList = DWindowManagerHelper::instance()->currentWorkspaceWindowIdList();
        foreignWindows.clear();
        for (quint32 currentId : qAsConst(currentIdList)) {
            foreignWindows.append(DForeignWindow::fromWinId(currentId));
        }
    }
    virtual void TearDown()
    {
        qDeleteAll(foreignWindows);
        foreignWindows.clear();
    }

    QList<DForeignWindow *> foreignWindows;
};

TEST_F(TDForeignWindow, wmClass)
{
    for (auto foreignWindow : qAsConst(foreignWindows))
        ASSERT_NE(foreignWindow->wmClass(), QString());
}

TEST_F(TDForeignWindow, pid)
{
    for (auto foreignWindow : qAsConst(foreignWindows))
        ASSERT_NE(foreignWindow->pid(), 0);
}

DGUI_END_NAMESPACE

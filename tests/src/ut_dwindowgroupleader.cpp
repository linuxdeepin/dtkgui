#include "test.h"
#include "dwindowgroupleader.h"
#include <QDebug>

#include <QTest>
#include <QWidget>

DGUI_USE_NAMESPACE


class TDWindowGroupLeader : public DTest
{
protected:
    void SetUp();
    void TearDown();

    DWindowGroupLeader *groupLeader;
    QWidget *tWidget_1;
    QWidget *tWidget_2;
};

void TDWindowGroupLeader::SetUp()
{
    groupLeader = new DWindowGroupLeader;
    tWidget_1 = new QWidget;
    tWidget_2 = new QWidget;

    tWidget_1->show();
    tWidget_2->show();
    ASSERT_TRUE(QTest::qWaitForWindowExposed(tWidget_1));
    ASSERT_TRUE(QTest::qWaitForWindowExposed(tWidget_2));
}

void TDWindowGroupLeader::TearDown()
{
    delete groupLeader;
    delete tWidget_1;
    delete tWidget_2;
}

TEST_F(TDWindowGroupLeader, testFunctions)
{
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
        return;

    // 测试多种情况添加window时数据是否正常
    groupLeader->addWindow(tWidget_1->windowHandle());
    groupLeader->addWindow(tWidget_2->windowHandle());

    ASSERT_TRUE(groupLeader->groupLeaderId() != 0);
    ASSERT_TRUE(groupLeader->clientLeaderId() != 0);

    groupLeader->removeWindow(tWidget_1->windowHandle());

    ASSERT_TRUE(groupLeader->groupLeaderId() != 0);
    ASSERT_TRUE(groupLeader->clientLeaderId() != 0);

    groupLeader->addWindow(tWidget_2->windowHandle());
    ASSERT_TRUE(groupLeader->groupLeaderId() != 0);
    ASSERT_TRUE(groupLeader->clientLeaderId() != 0);
}

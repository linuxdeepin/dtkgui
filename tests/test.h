#ifndef TEST_H
#define TEST_H

#include <gtest/gtest.h>

class DTest : public ::testing::Test
{
};

template<typename T>
class DTestWithParam : public ::testing::TestWithParam<T>
{
};
#endif // TEST_H

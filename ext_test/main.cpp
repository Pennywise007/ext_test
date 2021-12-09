#include "pch.h"

GTEST_API_ int main(int argc, char** argv)
{
    ::testing::FLAGS_gtest_catch_exceptions = false;

    ::testing::InitGoogleMock(&argc, argv);
    ::testing::InitGoogleTest(&argc, argv);

    setlocale(LC_ALL, "Russian");

    return RUN_ALL_TESTS();
}

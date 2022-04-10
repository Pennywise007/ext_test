#include "pch.h"

#include <ext/core.h>

GTEST_API_ int main(int argc, char** argv)
{
    ::testing::FLAGS_gtest_catch_exceptions = false;

    ::testing::InitGoogleMock(&argc, argv);
    ::testing::InitGoogleTest(&argc, argv);

    ext::core::Init();

    return RUN_ALL_TESTS();
}

#include <pch.h>
#include <vcruntime.h>

#include <ext/core/check.h>
#include <ext/trace/tracer.h>
#include <ext/error/exception.h>

TEST(TestException, CheckNested)
{
    try
    {
        try
        {
            throw ext::exception("Failed to do sth");
        }
        catch (...)
        {
            try
            {
                std::throw_with_nested(ext::exception(ext::source_location("File name", 11), "Job failed"));
            }
            catch (...)
            {
                std::throw_with_nested(std::runtime_error("Runtime error"));
            }
        }
    }
    catch (const std::exception&)
    {
        EXPECT_STREQ("Main error catched.\n\n"
                     "Exception: Runtime error\n"
                     "Job failed Exception At 'File name'(11).\n"
                     "Failed to do sth Exception", ext::ManageExceptionText("Main error catched").c_str());
    }
}
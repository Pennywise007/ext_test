#include <pch.h>
#include <vcruntime.h>

#include <ssh/core/check.h>
#include <ssh/trace/tracer.h>
#include <ssh/error/exception.h>

TEST(TestException, CheckNested)
{
    try
    {
        try
        {
            throw ssh::exception("Failed to do sth");
        }
        catch (...)
        {
            try
            {
                std::throw_with_nested(ssh::exception(ssh::source_location("File name", 11), "Job failed"));
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
                     "Failed to do sth Exception", ssh::ManageExceptionText("Main error catched").c_str());
    }
}
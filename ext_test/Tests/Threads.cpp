#include <pch.h>

#include <ext/thread/event.h>
#include <ext/thread/thread.h>

std::atomic_bool ThreadExecuted(false);
std::atomic_bool ThreadInterrupted(false);
std::atomic_bool ThreadRun(false);

void thread_function(const std::function<void()>& function)
{
    ThreadRun = true;

    ThreadExecuted = false;
    ThreadInterrupted = false;
    try
    {
        function();

        ThreadExecuted = true;
    }
    catch (const ext::thread::thread_interrupted&)
    {
        ThreadInterrupted = true;
    }
}

void join_thread_and_check(ext::thread& thread, bool interrupted)
{
    thread.join();
    EXPECT_TRUE(ThreadRun);

    if (interrupted)
    {
        EXPECT_FALSE(ThreadExecuted);
        EXPECT_TRUE(ThreadInterrupted);
    }
    else
    {
        EXPECT_TRUE(ThreadExecuted);
        EXPECT_FALSE(ThreadInterrupted);
    }

    ThreadRun = false;
    ThreadExecuted = false;
    ThreadInterrupted = false;
}

TEST(TestThreads, CheckInterruptionRequested)
{
    {
        ext::Event threadStartedEvent;
        threadStartedEvent.Create();
        ext::thread myThread(thread_function, [&threadStartedEvent]()
        {
            while (!ext::this_thread::interruption_requested())
            {
                threadStartedEvent.Set();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
        EXPECT_TRUE(threadStartedEvent.Wait());
        myThread.interrupt();
        join_thread_and_check(myThread, false);
    }
    {
        ext::thread myThread(thread_function, []()
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            EXPECT_FALSE(ext::this_thread::interruption_requested());
        });
        join_thread_and_check(myThread, false);
    }

    {
        ext::thread myThread(thread_function, []()
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            EXPECT_TRUE(ext::this_thread::interruption_requested());
        });
        myThread.interrupt();
        join_thread_and_check(myThread, false);
    }
}

TEST(TestThreads, CheckInteruptionPoint)
{
    {
        ext::thread myThread(thread_function, []()
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            ext::this_thread::interruption_point();
        });
        myThread.interrupt();
        join_thread_and_check(myThread, true);
    }
    {
        ext::thread myThread(thread_function, []()
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            ext::this_thread::interruption_point();
        });
        join_thread_and_check(myThread, false);
    }
}

TEST(TestThreads, CheckDetaching)
{
    ext::Event threadInterruptedAndDetached;
    threadInterruptedAndDetached.Create();
    std::atomic_bool interrupted = false;
    ext::thread myThread([&threadInterruptedAndDetached, &interrupted]()
    {
        EXPECT_TRUE(threadInterruptedAndDetached.Wait());
        try
        {
            ext::this_thread::interruption_point();
        }
        catch (const ext::thread::thread_interrupted&)
        {
            interrupted = true;
        }
    });
    myThread.interrupt();
    myThread.detach();
    threadInterruptedAndDetached.Set();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    EXPECT_TRUE(interrupted);
}

TEST(TestThreads, CheckOrdinarySleep)
{
    ext::thread myThread(thread_function, []() { ext::this_thread::sleep_for(std::chrono::seconds(3)); });
    std::this_thread::sleep_for(std::chrono::seconds(1));
    myThread.interrupt();
    join_thread_and_check(myThread, false);
}

TEST(TestThreads, CheckInterruptSleeping)
{
    ext::Event threadStartedEvent;
    threadStartedEvent.Create();
    ext::thread myThread(thread_function, [&]() { threadStartedEvent.Set(); ext::this_thread::interruptible_sleep_for(std::chrono::seconds(10)); });
    EXPECT_TRUE(threadStartedEvent.Wait());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    myThread.interrupt();
    join_thread_and_check(myThread, true);
}

TEST(TestThreads, CheckRunAndInterruptibleSleep)
{
    ext::thread myThread(thread_function, []() { ext::this_thread::interruptible_sleep_for(std::chrono::seconds(1)); });
    join_thread_and_check(myThread, false);

    myThread.run(thread_function, []() { ext::this_thread::interruptible_sleep_for(std::chrono::seconds(1)); });
    join_thread_and_check(myThread, false);
}
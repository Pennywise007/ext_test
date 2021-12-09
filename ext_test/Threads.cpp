#include <pch.h>

#include <ssh/thread/event.h>
#include <ssh/thread/thread.h>
#include <ssh/thread/thread_pool.h>

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
    catch (const ssh::thread::thread_interrupted&)
    {
        ThreadInterrupted = true;
    }
}

void join_thread_and_check(ssh::thread& thread, bool interrupted)
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
        ssh::Event threadStartedEvent;
        threadStartedEvent.Create();
        ssh::thread myThread(thread_function, [&threadStartedEvent]()
        {
            while (!ssh::this_thread::interruption_requested())
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
        ssh::thread myThread(thread_function, []()
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            EXPECT_FALSE(ssh::this_thread::interruption_requested());
        });
        join_thread_and_check(myThread, false);
    }

    {
        ssh::thread myThread(thread_function, []()
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            EXPECT_TRUE(ssh::this_thread::interruption_requested());
        });
        myThread.interrupt();
        join_thread_and_check(myThread, false);
    }
}

TEST(TestThreads, CheckInteruptionPoint)
{
    {
        ssh::thread myThread(thread_function, []()
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            ssh::this_thread::interruption_point();
        });
        myThread.interrupt();
        join_thread_and_check(myThread, true);
    }
    {
        ssh::thread myThread(thread_function, []()
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            ssh::this_thread::interruption_point();
        });
        join_thread_and_check(myThread, false);
    }
}

TEST(TestThreads, CheckDetaching)
{
    ssh::Event threadInterruptedAndDetached;
    threadInterruptedAndDetached.Create();
    std::atomic_bool interrupted = false;
    ssh::thread myThread([&threadInterruptedAndDetached, &interrupted]()
    {
        EXPECT_TRUE(threadInterruptedAndDetached.Wait());
        try
        {
            ssh::this_thread::interruption_point();
        }
        catch (const ssh::thread::thread_interrupted&)
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
    ssh::thread myThread(thread_function, []() { ssh::this_thread::sleep_for(std::chrono::seconds(3)); });
    std::this_thread::sleep_for(std::chrono::seconds(1));
    myThread.interrupt();
    join_thread_and_check(myThread, false);
}

TEST(TestThreads, CheckInterruptSleeping)
{
    ssh::Event threadStartedEvent;
    threadStartedEvent.Create();
    ssh::thread myThread(thread_function, [&]() { threadStartedEvent.Set(); ssh::this_thread::interruptible_sleep_for(std::chrono::seconds(10)); });
    EXPECT_TRUE(threadStartedEvent.Wait());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    myThread.interrupt();
    join_thread_and_check(myThread, true);
}

TEST(TestThreads, CheckRunAndInterruptibleSleep)
{
    ssh::thread myThread(thread_function, []() { ssh::this_thread::interruptible_sleep_for(std::chrono::seconds(1)); });
    join_thread_and_check(myThread, false);

    myThread.run(thread_function, []() { ssh::this_thread::interruptible_sleep_for(std::chrono::seconds(1)); });
    join_thread_and_check(myThread, false);
}
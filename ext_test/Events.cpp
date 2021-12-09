#include <pch.h>

#include <ssh/thread/event.h>

TEST(TestEvent, CheckRising)
{
    ssh::Event event;
    event.Create();
    EXPECT_TRUE(event);
    std::thread myThread([&event]()
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        event.Set();
    });
    EXPECT_TRUE(event.Wait(std::chrono::seconds(2)));
    EXPECT_TRUE(event);

    std::thread myThread2([&event]()
    {
        event.Set();
    });

    myThread2.join();
    EXPECT_TRUE(event.Wait(std::chrono::seconds(0)));

    event.Destroy();
    EXPECT_FALSE(event);

    myThread.join();
}

TEST(TestEvent, CheckManualReset)
{
    ssh::Event event;
    std::thread myThread([&event]()
    {
        event.Create(false);
    });
    myThread.join();
    EXPECT_TRUE(event);
}

TEST(TestEvent, CheckWaitAfterSet)
{
    ssh::Event event;
    event.Create();
    event.Set();

    EXPECT_TRUE(event.Wait());
}
#include <pch.h>
#include <optional>

#include <ext/thread/scheduller.h>

TEST(TestSchedule, CheckSchedulingTasks)
{
    ext::Scheduler& scheduler = ext::Scheduler::GlobalInstance();

    bool executed = false;
    std::chrono::high_resolution_clock::time_point callTime = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(900);
    const auto taskIdAtTime = scheduler.SubscribeTaskAtTime(
        [&executed, &callTime]()
        {
            executed = true;

            const auto currentTime = std::chrono::high_resolution_clock::now();

            EXPECT_TRUE(currentTime >= callTime);
            EXPECT_TRUE(currentTime - callTime < std::chrono::milliseconds(1));
        },
        callTime);
    EXPECT_EQ(taskIdAtTime, 0);

    UINT callCount = 0;
    std::optional<std::chrono::high_resolution_clock::time_point> firstCall;
    const auto taskIdByPeriod = scheduler.SubscribeTaskByPeriod(
        [&callCount, &firstCall]()
        {
            ++callCount;
            const auto now = std::chrono::high_resolution_clock::now();
            if (!firstCall.has_value())
                firstCall = now;
            else
            {
                const auto expectedCallTime = *firstCall + std::chrono::milliseconds(400);
                EXPECT_TRUE(now >= expectedCallTime);
                EXPECT_TRUE(now - expectedCallTime < std::chrono::milliseconds(1));
            }
        },
        std::chrono::milliseconds(400));
    EXPECT_EQ(taskIdByPeriod, 1);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    EXPECT_FALSE(scheduler.IsTaskExists(taskIdAtTime));
    EXPECT_EQ(executed, true);

    EXPECT_TRUE(scheduler.IsTaskExists(taskIdByPeriod));
    scheduler.RemoveTask(taskIdByPeriod);
    EXPECT_FALSE(scheduler.IsTaskExists(taskIdByPeriod));
    EXPECT_EQ(callCount, 2);
}
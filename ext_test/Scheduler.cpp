#include <pch.h>
#include <optional>

#include <ext/thread/scheduler.h>

TEST(TestSchedule, CheckSchedulingTasks)
{
    ext::Scheduler& scheduler = ext::Scheduler::GlobalInstance();

    bool executed = false;
    std::chrono::system_clock::time_point callTime = std::chrono::system_clock::now() + std::chrono::milliseconds(1000);
    const auto taskIdAtTime = scheduler.SubscribeTaskAtTime(
        [&executed, &callTime]()
        {
            const auto currentTime = std::chrono::system_clock::now();

            executed = true;

            EXPECT_TRUE(currentTime >= callTime);
            EXPECT_LE(std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - callTime).count(), 30);
        },
        callTime);
    EXPECT_EQ(taskIdAtTime, 0);

    UINT callCount = 0;
    std::optional<std::chrono::system_clock::time_point> firstCall;
    const auto taskIdByPeriod = scheduler.SubscribeTaskByPeriod(
        [&callCount, &firstCall]()
        {
            const auto now = std::chrono::system_clock::now();
            ++callCount;
            if (!firstCall.has_value())
                firstCall = now;
            else
            {
                const auto expectedCallTime = *firstCall + (callCount - 1) * std::chrono::milliseconds(400);
                EXPECT_LE(abs(std::chrono::duration_cast<std::chrono::milliseconds>(now - expectedCallTime).count()), 30);
            }
        },
        std::chrono::milliseconds(400));
    EXPECT_EQ(taskIdByPeriod, 1);

    std::this_thread::sleep_for(std::chrono::milliseconds(850));

    EXPECT_TRUE(scheduler.IsTaskExists(taskIdAtTime));
    EXPECT_EQ(executed, false);

    EXPECT_TRUE(scheduler.IsTaskExists(taskIdByPeriod));
    EXPECT_EQ(callCount, 2);

    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    EXPECT_FALSE(scheduler.IsTaskExists(taskIdAtTime));
    EXPECT_EQ(executed, true);

    EXPECT_TRUE(scheduler.IsTaskExists(taskIdByPeriod));
    scheduler.RemoveTask(taskIdByPeriod);
    EXPECT_FALSE(scheduler.IsTaskExists(taskIdByPeriod));
    EXPECT_EQ(callCount, 3);
}

TEST(TestSchedule, CheckSchedulingTask_AtTimeExists)
{
    ext::Scheduler& scheduler = ext::Scheduler::GlobalInstance();

    bool executed = false;
    std::chrono::system_clock::time_point callTime = std::chrono::system_clock::now() + std::chrono::milliseconds(100);
    ext::TaskId taskId;
    taskId = scheduler.SubscribeTaskAtTime(
        [&]()
        {
            EXPECT_FALSE(scheduler.IsTaskExists(taskId));
            executed = true;
        },
        callTime);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_TRUE(executed);
}
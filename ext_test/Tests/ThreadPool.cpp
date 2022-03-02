#include <pch.h>

#include <ext/thread/event.h>
#include <ext/thread/thread_pool.h>

TEST(TestThreadPool, CheckAddAndExecutingTasks)
{
    std::mutex listMutex;
    std::set<ext::task::TaskId, ext::task::TaskIdComparer> taskList;

    std::atomic_int runTaskCount = 0;

    ext::thread_pool threadPool([&taskList, &listMutex](const ext::task::TaskId& taskId)
    {
        std::scoped_lock lock(listMutex);
        EXPECT_NE(taskList.find(taskId), taskList.end()) << "Unknown task id";
        taskList.erase(taskId);
    });

    {
        std::scoped_lock lock(listMutex);
        const auto maxThreads = std::thread::hardware_concurrency();
        for (auto i = maxThreads; i != 0; --i)
        {
            EXPECT_TRUE(taskList.emplace(threadPool.add_task([&runTaskCount]()
            {
                ++runTaskCount;
                std::this_thread::sleep_for(std::chrono::seconds(3));
                --runTaskCount;
            })).second) << "Duplicate of task id";
        }
        EXPECT_EQ(taskList.size(), maxThreads);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        EXPECT_EQ(runTaskCount, maxThreads);
    }

    threadPool.wait_for_tasks();
    EXPECT_TRUE(taskList.empty()) << "not all tasks finish callbacks called";
    EXPECT_EQ(runTaskCount, 0) << "not all task executed";
}

TEST(TestThreadPool, CheckRemovingTasks)
{
    {
        std::atomic_uint executedTasksCount = 0;
        ext::thread_pool threadPool([&executedTasksCount](const ext::task::TaskId& taskId)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            ++executedTasksCount;
        }, 1);

        for (int i = 0; i < 5; ++i)
        {
            const auto taskId = threadPool.add_task([]()
            {});
            if (i != 0)
                threadPool.erase_task(taskId);
        }
        threadPool.wait_for_tasks();
        EXPECT_EQ(executedTasksCount, 1);
    }
    {
        std::atomic_uint executedTasksCount = 0;
        ext::thread_pool threadPool([&executedTasksCount](const ext::task::TaskId& taskId)
        {
            ++executedTasksCount;
        }, 1);

        for (int i = 0; i < 5; ++i)
        {
            const auto taskId = threadPool.add_task([]()
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            });
            if (i != 0)
                threadPool.erase_task(taskId);
        }
        threadPool.wait_for_tasks();
        EXPECT_EQ(executedTasksCount, 1);
    }
}

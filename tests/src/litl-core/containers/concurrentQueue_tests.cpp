#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <thread>

#include "litl-core/containers/concurrentQueue.hpp"

TEST_CASE("Push-Pop Single-Threaded", "[core::containers::concurrentQueue]")
{
    LITL::Core::Containers::ConcurrentQueue<uint32_t> queue{};

    REQUIRE(queue.size() == 0);

    queue.push(55);

    REQUIRE(queue.size() == 1);
    REQUIRE(queue.pop() == 55);
    REQUIRE(queue.size() == 0);
    REQUIRE(queue.pop() == std::nullopt);
}

TEST_CASE("Push-Peek Single-Threaded", "[core::containers::concurrentQueue]")
{
    LITL::Core::Containers::ConcurrentQueue<uint32_t> queue{};

    REQUIRE(queue.size() == 0);
    REQUIRE(queue.peek() == std::nullopt);

    queue.push(55);

    REQUIRE(queue.size() == 1);
    REQUIRE(queue.peek() == 55);
    REQUIRE(queue.size() == 1);
    REQUIRE(queue.pop() == 55);
    REQUIRE(queue.peek() == std::nullopt);
    REQUIRE(queue.size() == 0);
}

TEST_CASE("Push-Clear Single-Threaded", "[core::containers::concurrentQueue]")
{
    LITL::Core::Containers::ConcurrentQueue<uint32_t> queue{};

    for (int i = 0; i < 50; ++i)
    {
        queue.push(i);
    }

    REQUIRE(queue.size() == 50);
    REQUIRE(queue.peek() == 0);     // queue is FIFO

    queue.clear();

    REQUIRE(queue.size() == 0);
    REQUIRE(queue.peek() == std::nullopt);
}

namespace PushPopMultithreadedTest
{
    std::mutex mutex;
    std::condition_variable_any condition;

    /// <summary>
    /// Adds items to the queue until the queue has at least 100 items in it.
    /// </summary>
    /// <param name="stopToken"></param>
    /// <param name="queue"></param>
    /// <param name="workerId"></param>
    void pushThread(std::stop_token stopToken, LITL::Core::Containers::ConcurrentQueue<uint32_t>& queue, uint32_t workerId)
    {
        int iterations = 1000;

        while (!stopToken.stop_requested() && queue.size() < 100 && iterations > 0)
        {
            queue.push(workerId);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            iterations--;
        }
    }

    /// <summary>
    /// Waits for the queue to have at least 100 items in it and the removes items until it has exactly 50.
    /// </summary>
    /// <param name="stopToken"></param>
    /// <param name="queue"></param>
    /// <param name="workerId"></param>
    void popThread(std::stop_token stopToken, LITL::Core::Containers::ConcurrentQueue<uint32_t>& queue, uint32_t workerId)
    {
        int iterations = 1000;
        bool readyToPop = false;

        while (!stopToken.stop_requested() && iterations > 0)
        {
            if (!readyToPop && (queue.size() >= 100))
            {
                readyToPop = true;
            }

            if (readyToPop)
            {
                if (queue.size() == 50)
                {
                    break;
                }
                else
                {
                    queue.pop();
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            iterations--;
        }

        condition.notify_all();
    }
}


TEST_CASE("Push-Pop Multi-Threaded", "[core::containers::concurrentQueue]")
{
    LITL::Core::Containers::ConcurrentQueue<uint32_t> queue{};

    std::jthread thread0(PushPopMultithreadedTest::pushThread, std::ref(queue), 0);
    std::jthread thread1(PushPopMultithreadedTest::pushThread, std::ref(queue), 1);
    std::jthread thread2(PushPopMultithreadedTest::popThread, std::ref(queue), 2);
    std::jthread thread3(PushPopMultithreadedTest::popThread, std::ref(queue), 3);

    {
        std::unique_lock<std::mutex> lock(PushPopMultithreadedTest::mutex);
        PushPopMultithreadedTest::condition.wait(lock);
    }

    REQUIRE(queue.size() == 50);
}
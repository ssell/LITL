#include <array>
#include <vector>

#include "tests.hpp"
#include "litl-core/services/serviceCollection.hpp"
#include "litl-platform/window/window.hpp"
#include "litl-platform/window/windowCallbacks.hpp"

namespace litl::tests
{
    namespace
    {
        constexpr std::array<WindowCallbackType, 5> AllWindowCallbackTypes{
            WindowCallbackType::Opened,
            WindowCallbackType::Closed,
            WindowCallbackType::Minimized,
            WindowCallbackType::Restored,
            WindowCallbackType::Resized
        };

        struct WindowCallbacksFixture
        {
            ServiceCollection collection{};
            std::shared_ptr<ServiceProvider> services{ collection.build() };
            WindowCallbacks callbacks{};
            Window window{};                                                // never setup; destroy() is a no-op without ops

            void process()
            {
                callbacks.processDeferredCommands();
            }

            WindowCallbackHandle registerCallback(WindowCallbackType type, OnWindowEventFunc&& func, WindowCallbackPriority priority = WindowCallbackPriority::Medium)
            {
                switch (type)
                {
                case WindowCallbackType::Opened:    return callbacks.registerWindowOpenedCallback(std::move(func), priority);
                case WindowCallbackType::Closed:    return callbacks.registerWindowClosedCallback(std::move(func), priority);
                case WindowCallbackType::Minimized: return callbacks.registerWindowMinimizedCallback(std::move(func), priority);
                case WindowCallbackType::Restored:  return callbacks.registerWindowRestoredCallback(std::move(func), priority);
                case WindowCallbackType::Resized:   return callbacks.registerWindowResizedCallback(std::move(func), priority);
                default:                            return {};
                }
            }

            void invoke(WindowCallbackType type)
            {
                const auto auth = Authority<Window>::mint();

                switch (type)
                {
                case WindowCallbackType::Opened:    callbacks.invokeWindowOpenedEvent(auth, *services, window); break;
                case WindowCallbackType::Closed:    callbacks.invokeWindowClosedEvent(auth, *services, window); break;
                case WindowCallbackType::Minimized: callbacks.invokeWindowMinimizedEvent(auth, *services, window); break;
                case WindowCallbackType::Restored:  callbacks.invokeWindowRestoredEvent(auth, *services, window); break;
                case WindowCallbackType::Resized:   callbacks.invokeWindowResizedEvent(auth, *services, window); break;
                default: break;
                }
            }

            void resize()
            {
                invoke(WindowCallbackType::Resized);
            }
        };

        /// <summary>
        /// Window callback that increments a counter and returns the specified result.
        /// </summary>
        OnWindowEventFunc countingCallback(uint32_t& counter, WindowCallbackResult result = WindowCallbackResult::Continue)
        {
            return [&counter, result](ServiceProvider&, Window const&)
                {
                    ++counter;
                    return result;
                };
        }

        /// <summary>
        /// Window callback that appends its tag to an ordering log and returns the specified result.
        /// </summary>
        OnWindowEventFunc orderedCallback(std::vector<uint32_t>& log, uint32_t tag, WindowCallbackResult result = WindowCallbackResult::Continue)
        {
            return [&log, tag, result](ServiceProvider&, Window const&)
                {
                    log.push_back(tag);
                    return result;
                };
        }

        constexpr WindowCallbackPriority offsetPriority(WindowCallbackPriority priority, int32_t offset) noexcept
        {
            return WindowCallbackPriority{ static_cast<uint32_t>(static_cast<int32_t>(priority) + offset) };
        }
    }

    // -------------------------------------------------------------------------------------
    // Registration
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "registration is deferred until processed", "[platform::windowCallbacks]")
    {
        uint32_t count = 0u;
        const auto handle = callbacks.registerWindowResizedCallback(countingCallback(count));

        REQUIRE(handle.id != Constants::uint32_null_index);
        REQUIRE(handle.type == WindowCallbackType::Resized);

        resize();
        REQUIRE(count == 0u);

        process();
        resize();
        REQUIRE(count == 1u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "handles carry the registered event type", "[platform::windowCallbacks]")
    {
        uint32_t count = 0u;

        for (const auto type : AllWindowCallbackTypes)
        {
            const auto handle = registerCallback(type, countingCallback(count));
            REQUIRE(handle.type == type);
        }
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "handles are unique", "[platform::windowCallbacks]")
    {
        uint32_t count = 0u;
        const auto a = callbacks.registerWindowResizedCallback(countingCallback(count));
        const auto b = callbacks.registerWindowResizedCallback(countingCallback(count));
        const auto c = callbacks.registerWindowOpenedCallback(countingCallback(count));

        REQUIRE(a.id != b.id);
        REQUIRE(a.id != c.id);
        REQUIRE(b.id != c.id);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "callbacks only fire for their event type", "[platform::windowCallbacks]")
    {
        std::array<uint32_t, AllWindowCallbackTypes.size()> counts{};

        for (size_t i = 0u; i < AllWindowCallbackTypes.size(); ++i)
        {
            registerCallback(AllWindowCallbackTypes[i], countingCallback(counts[i]));
        }

        process();

        for (size_t i = 0u; i < AllWindowCallbackTypes.size(); ++i)
        {
            invoke(AllWindowCallbackTypes[i]);

            for (size_t j = 0u; j < AllWindowCallbackTypes.size(); ++j)
            {
                REQUIRE(counts[j] == ((j <= i) ? 1u : 0u));
            }
        }
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "callback receives the invoking window", "[platform::windowCallbacks]")
    {
        Window const* received = nullptr;

        callbacks.registerWindowResizedCallback([&received](ServiceProvider&, Window const& w)
            {
                received = &w;
                return WindowCallbackResult::Continue;
            });
        process();

        resize();

        REQUIRE(received == &window);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "invoking with no callbacks is a no-op", "[platform::windowCallbacks]")
    {
        process();

        for (const auto type : AllWindowCallbackTypes)
        {
            invoke(type);
        }

        SUCCEED();
    }
    LITL_END_TEST_CASE

        // -------------------------------------------------------------------------------------
        // Propagation
        // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "continue propagates to all callbacks", "[platform::windowCallbacks]")
    {
        uint32_t first = 0u;
        uint32_t second = 0u;

        callbacks.registerWindowResizedCallback(countingCallback(first));
        callbacks.registerWindowResizedCallback(countingCallback(second));
        process();

        resize();

        REQUIRE(first == 1u);
        REQUIRE(second == 1u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "consume stops propagation", "[platform::windowCallbacks]")
    {
        uint32_t first = 0u;
        uint32_t second = 0u;

        callbacks.registerWindowResizedCallback(countingCallback(first, WindowCallbackResult::Consume));
        callbacks.registerWindowResizedCallback(countingCallback(second));
        process();

        resize();

        REQUIRE(first == 1u);
        REQUIRE(second == 0u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "consume does not disable the callback", "[platform::windowCallbacks]")
    {
        uint32_t consumer = 0u;
        uint32_t lower = 0u;

        callbacks.registerWindowResizedCallback(countingCallback(consumer, WindowCallbackResult::Consume), WindowCallbackPriority::High);
        callbacks.registerWindowResizedCallback(countingCallback(lower), WindowCallbackPriority::Low);
        process();

        resize();
        process();
        resize();

        REQUIRE(consumer == 2u);
        REQUIRE(lower == 0u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "consume only affects the consumed event type", "[platform::windowCallbacks]")
    {
        uint32_t resizedConsumer = 0u;
        uint32_t resizedLower = 0u;
        uint32_t restored = 0u;

        callbacks.registerWindowResizedCallback(countingCallback(resizedConsumer, WindowCallbackResult::Consume));
        callbacks.registerWindowResizedCallback(countingCallback(resizedLower));
        callbacks.registerWindowRestoredCallback(countingCallback(restored));
        process();

        resize();
        invoke(WindowCallbackType::Restored);

        REQUIRE(resizedConsumer == 1u);
        REQUIRE(resizedLower == 0u);
        REQUIRE(restored == 1u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "callbacks are invoked in registration order", "[platform::windowCallbacks]")
    {
        std::vector<uint32_t> log;

        callbacks.registerWindowResizedCallback(orderedCallback(log, 1u));
        callbacks.registerWindowResizedCallback(orderedCallback(log, 2u));
        callbacks.registerWindowResizedCallback(orderedCallback(log, 3u));
        process();

        resize();

        REQUIRE(log == std::vector<uint32_t>{ 1u, 2u, 3u });
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "repeated events in one frame dispatch each time", "[platform::windowCallbacks]")
    {
        uint32_t count = 0u;

        callbacks.registerWindowResizedCallback(countingCallback(count));
        process();

        resize();
        resize();
        resize();

        REQUIRE(count == 3u);
    }
    LITL_END_TEST_CASE

        // -------------------------------------------------------------------------------------
        // Self-removal
        // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "continue and remove fires once and still propagates", "[platform::windowCallbacks]")
    {
        uint32_t oneShot = 0u;
        uint32_t persistent = 0u;

        callbacks.registerWindowResizedCallback(countingCallback(oneShot, WindowCallbackResult::ContinueAndRemove));
        callbacks.registerWindowResizedCallback(countingCallback(persistent));
        process();

        resize();

        REQUIRE(oneShot == 1u);
        REQUIRE(persistent == 1u);

        process();
        resize();

        REQUIRE(oneShot == 1u);
        REQUIRE(persistent == 2u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "consume and remove fires once and stops propagation for that dispatch", "[platform::windowCallbacks]")
    {
        uint32_t oneShot = 0u;
        uint32_t persistent = 0u;

        callbacks.registerWindowResizedCallback(countingCallback(oneShot, WindowCallbackResult::ConsumeAndRemove));
        callbacks.registerWindowResizedCallback(countingCallback(persistent));
        process();

        resize();

        REQUIRE(oneShot == 1u);
        REQUIRE(persistent == 0u);

        process();
        resize();

        REQUIRE(oneShot == 1u);
        REQUIRE(persistent == 1u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "removed callback does not fire again before processing", "[platform::windowCallbacks]")
    {
        uint32_t oneShot = 0u;

        callbacks.registerWindowResizedCallback(countingCallback(oneShot, WindowCallbackResult::ContinueAndRemove));
        process();

        resize();
        resize();                                                   // same frame, removal not yet processed

        REQUIRE(oneShot == 1u);
    }
    LITL_END_TEST_CASE

        // -------------------------------------------------------------------------------------
        // Unregistration
        // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "unregister removes the callback for every event type", "[platform::windowCallbacks]")
    {
        std::array<uint32_t, AllWindowCallbackTypes.size()> counts{};
        std::array<WindowCallbackHandle, AllWindowCallbackTypes.size()> handles{};

        for (size_t i = 0u; i < AllWindowCallbackTypes.size(); ++i)
        {
            handles[i] = registerCallback(AllWindowCallbackTypes[i], countingCallback(counts[i]));
        }

        process();

        for (const auto handle : handles)
        {
            callbacks.unregisterCallback(handle);
        }

        process();

        for (const auto type : AllWindowCallbackTypes)
        {
            invoke(type);
        }

        for (const auto count : counts)
        {
            REQUIRE(count == 0u);
        }
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "unregister is deferred until processed", "[platform::windowCallbacks]")
    {
        uint32_t count = 0u;
        const auto handle = callbacks.registerWindowResizedCallback(countingCallback(count));
        process();

        callbacks.unregisterCallback(handle);
        resize();
        REQUIRE(count == 1u);

        process();
        resize();
        REQUIRE(count == 1u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "unregister only removes the matching callback", "[platform::windowCallbacks]")
    {
        uint32_t removed = 0u;
        uint32_t kept = 0u;

        const auto handle = callbacks.registerWindowResizedCallback(countingCallback(removed));
        callbacks.registerWindowResizedCallback(countingCallback(kept));
        process();

        callbacks.unregisterCallback(handle);
        process();
        resize();

        REQUIRE(removed == 0u);
        REQUIRE(kept == 1u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "unregister with a mismatched type does not remove the callback", "[platform::windowCallbacks]")
    {
        uint32_t count = 0u;
        auto handle = callbacks.registerWindowResizedCallback(countingCallback(count));
        process();

        handle.type = WindowCallbackType::Opened;
        callbacks.unregisterCallback(handle);
        process();
        resize();

        REQUIRE(count == 1u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "register and unregister in the same batch", "[platform::windowCallbacks]")
    {
        uint32_t count = 0u;
        const auto handle = callbacks.registerWindowResizedCallback(countingCallback(count));

        callbacks.unregisterCallback(handle);
        process();
        resize();

        REQUIRE(count == 0u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "unregister with a null handle is a no-op", "[platform::windowCallbacks]")
    {
        uint32_t count = 0u;
        callbacks.registerWindowResizedCallback(countingCallback(count));
        process();

        callbacks.unregisterCallback(WindowCallbackHandle{});
        process();
        resize();

        REQUIRE(count == 1u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "unregistering twice is harmless", "[platform::windowCallbacks]")
    {
        uint32_t removed = 0u;
        uint32_t kept = 0u;

        const auto handle = callbacks.registerWindowResizedCallback(countingCallback(removed));
        callbacks.registerWindowResizedCallback(countingCallback(kept));
        process();

        callbacks.unregisterCallback(handle);
        callbacks.unregisterCallback(handle);
        process();
        callbacks.unregisterCallback(handle);
        process();
        resize();

        REQUIRE(removed == 0u);
        REQUIRE(kept == 1u);
    }
    LITL_END_TEST_CASE

        // -------------------------------------------------------------------------------------
        // Re-entrancy
        // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "callback registered during dispatch fires next frame", "[platform::windowCallbacks]")
    {
        uint32_t lateCount = 0u;

        callbacks.registerWindowResizedCallback([this, &lateCount](ServiceProvider&, Window const&)
            {
                callbacks.registerWindowResizedCallback(countingCallback(lateCount));
                return WindowCallbackResult::ContinueAndRemove;
            });
        process();

        resize();
        REQUIRE(lateCount == 0u);

        process();
        resize();
        REQUIRE(lateCount == 1u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "callback unregistering itself during dispatch is safe", "[platform::windowCallbacks]")
    {
        uint32_t count = 0u;
        WindowCallbackHandle self{};

        self = callbacks.registerWindowResizedCallback([this, &self, &count](ServiceProvider&, Window const&)
            {
                ++count;
                callbacks.unregisterCallback(self);
                return WindowCallbackResult::Continue;
            });
        process();

        resize();
        process();
        resize();

        REQUIRE(count == 1u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "callback unregistering another during dispatch takes effect next frame", "[platform::windowCallbacks]")
    {
        uint32_t victim = 0u;
        WindowCallbackHandle victimHandle{};

        callbacks.registerWindowResizedCallback([this, &victimHandle](ServiceProvider&, Window const&)
            {
                callbacks.unregisterCallback(victimHandle);
                return WindowCallbackResult::Continue;
            }, WindowCallbackPriority::High);

        victimHandle = callbacks.registerWindowResizedCallback(countingCallback(victim), WindowCallbackPriority::Low);
        process();

        resize();
        REQUIRE(victim == 1u);                                      // removal is deferred, so it still runs this frame

        process();
        resize();
        REQUIRE(victim == 1u);
    }
    LITL_END_TEST_CASE

        // -------------------------------------------------------------------------------------
        // Priority
        // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "higher priority runs first regardless of registration order", "[platform::windowCallbacks]")
    {
        std::vector<uint32_t> log;

        callbacks.registerWindowResizedCallback(orderedCallback(log, 1u), WindowCallbackPriority::Low);
        callbacks.registerWindowResizedCallback(orderedCallback(log, 3u), WindowCallbackPriority::High);
        callbacks.registerWindowResizedCallback(orderedCallback(log, 2u), WindowCallbackPriority::Medium);
        process();

        resize();

        REQUIRE(log == std::vector<uint32_t>{ 3u, 2u, 1u });
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "equal priority preserves registration order", "[platform::windowCallbacks]")
    {
        constexpr uint32_t CallbackCount = 64u;

        std::vector<uint32_t> log;
        std::vector<uint32_t> expected;

        for (uint32_t i = 0u; i < CallbackCount; ++i)
        {
            callbacks.registerWindowResizedCallback(orderedCallback(log, i), WindowCallbackPriority::Medium);
            expected.push_back(i);
        }

        process();
        resize();

        REQUIRE(log == expected);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "ties keep registration order within each priority", "[platform::windowCallbacks]")
    {
        std::vector<uint32_t> log;

        callbacks.registerWindowResizedCallback(orderedCallback(log, 1u), WindowCallbackPriority::Medium);
        callbacks.registerWindowResizedCallback(orderedCallback(log, 2u), WindowCallbackPriority::High);
        callbacks.registerWindowResizedCallback(orderedCallback(log, 3u), WindowCallbackPriority::Medium);
        callbacks.registerWindowResizedCallback(orderedCallback(log, 4u), WindowCallbackPriority::High);
        process();

        resize();

        REQUIRE(log == std::vector<uint32_t>{ 2u, 4u, 1u, 3u });
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "default priority is medium", "[platform::windowCallbacks]")
    {
        std::vector<uint32_t> log;

        callbacks.registerWindowResizedCallback(orderedCallback(log, 1u), offsetPriority(WindowCallbackPriority::Medium, -1));
        callbacks.registerWindowResizedCallback(orderedCallback(log, 2u), WindowCallbackPriority::Medium);
        callbacks.registerWindowResizedCallback(orderedCallback(log, 3u));                                      // default
        callbacks.registerWindowResizedCallback(orderedCallback(log, 4u), offsetPriority(WindowCallbackPriority::Medium, 1));
        process();

        resize();

        REQUIRE(log == std::vector<uint32_t>{ 4u, 2u, 3u, 1u });
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "custom priorities sort between named levels", "[platform::windowCallbacks]")
    {
        std::vector<uint32_t> log;

        callbacks.registerWindowResizedCallback(orderedCallback(log, 1u), WindowCallbackPriority::Low);
        callbacks.registerWindowResizedCallback(orderedCallback(log, 3u), WindowCallbackPriority::Medium);
        callbacks.registerWindowResizedCallback(orderedCallback(log, 2u), offsetPriority(WindowCallbackPriority::Low, 1));
        callbacks.registerWindowResizedCallback(orderedCallback(log, 4u), offsetPriority(WindowCallbackPriority::High, 1));
        process();

        resize();

        REQUIRE(log == std::vector<uint32_t>{ 4u, 3u, 2u, 1u });
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "higher priority consume blocks lower priorities", "[platform::windowCallbacks]")
    {
        std::vector<uint32_t> log;

        callbacks.registerWindowResizedCallback(orderedCallback(log, 1u), WindowCallbackPriority::Low);
        callbacks.registerWindowResizedCallback(orderedCallback(log, 2u, WindowCallbackResult::Consume), WindowCallbackPriority::Medium);
        callbacks.registerWindowResizedCallback(orderedCallback(log, 3u), WindowCallbackPriority::High);
        process();

        resize();

        REQUIRE(log == std::vector<uint32_t>{ 3u, 2u });
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "later registration with higher priority is inserted ahead", "[platform::windowCallbacks]")
    {
        std::vector<uint32_t> log;

        callbacks.registerWindowResizedCallback(orderedCallback(log, 1u), WindowCallbackPriority::Medium);
        callbacks.registerWindowResizedCallback(orderedCallback(log, 2u), WindowCallbackPriority::Medium);
        process();

        resize();
        REQUIRE(log == std::vector<uint32_t>{ 1u, 2u });

        log.clear();
        callbacks.registerWindowResizedCallback(orderedCallback(log, 3u), WindowCallbackPriority::High);
        callbacks.registerWindowResizedCallback(orderedCallback(log, 4u), WindowCallbackPriority::Low);
        process();

        resize();
        REQUIRE(log == std::vector<uint32_t>{ 3u, 1u, 2u, 4u });
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "unregistering preserves priority order", "[platform::windowCallbacks]")
    {
        std::vector<uint32_t> log;

        callbacks.registerWindowResizedCallback(orderedCallback(log, 1u), WindowCallbackPriority::Low);
        const auto middle = callbacks.registerWindowResizedCallback(orderedCallback(log, 2u), WindowCallbackPriority::Medium);
        callbacks.registerWindowResizedCallback(orderedCallback(log, 3u), WindowCallbackPriority::High);
        process();

        callbacks.unregisterCallback(middle);
        process();
        resize();

        REQUIRE(log == std::vector<uint32_t>{ 3u, 1u });
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "one-shot high priority callback leaves order intact", "[platform::windowCallbacks]")
    {
        std::vector<uint32_t> log;

        callbacks.registerWindowResizedCallback(orderedCallback(log, 1u), WindowCallbackPriority::Low);
        callbacks.registerWindowResizedCallback(orderedCallback(log, 2u, WindowCallbackResult::ContinueAndRemove), WindowCallbackPriority::High);
        callbacks.registerWindowResizedCallback(orderedCallback(log, 3u), WindowCallbackPriority::Medium);
        process();

        resize();
        REQUIRE(log == std::vector<uint32_t>{ 2u, 3u, 1u });

        log.clear();
        process();
        resize();
        REQUIRE(log == std::vector<uint32_t>{ 3u, 1u });
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(WindowCallbacksFixture, "priority applies to every event type", "[platform::windowCallbacks]")
    {
        for (const auto type : AllWindowCallbackTypes)
        {
            std::vector<uint32_t> log;

            const auto low = registerCallback(type, orderedCallback(log, 1u), WindowCallbackPriority::Low);
            const auto high = registerCallback(type, orderedCallback(log, 2u), WindowCallbackPriority::High);
            process();

            invoke(type);
            REQUIRE(log == std::vector<uint32_t>{ 2u, 1u });

            callbacks.unregisterCallback(low);                      // clean up before log goes out of scope
            callbacks.unregisterCallback(high);
            process();
        }
    }
    LITL_END_TEST_CASE
}
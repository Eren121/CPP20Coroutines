#include <catch2/catch.hpp>
#include "debug.hpp"
#include "task.hpp"

using namespace coro;

namespace
{
    task<void> noop()
    {
        co_return;
    }

    task<void> never()
    {
        co_await std::suspend_never{};
    }

    task<void> always()
    {
        co_await std::suspend_always{};
    }

    task<void> always2()
    {
        co_await always();
        co_await always();
    }

    template<task<void>(*...step)()>
    task<void> callSteps()
    {
        ((co_await step()), ...);
    }

    TEST_CASE("task<void>")
    {
        SECTION("noop")
        {
            auto handle = noop();
            REQUIRE(!handle());
        }

        SECTION("never")
        {
            auto handle = never();
            REQUIRE(!handle());
        }

        SECTION("always")
        {
            auto handle = always();
            handle();
            REQUIRE(!handle());
        }

        SECTION("inner task does not wait")
        {
            {
                auto handle = callSteps<noop>();
                REQUIRE(!handle()); // The coroutine should no co_a'wait the noop, so only one call should end the coroutine
            }

            {
                auto handle = callSteps<noop, noop>();
                REQUIRE(!handle()); // Same whether regardless the count call of co_await
            }
            {
                auto handle = callSteps<always>();
                handle();
                REQUIRE(!handle());
            }
            {
                auto handle = callSteps<always, never>();
                handle();
                REQUIRE(!handle());
            }

            SECTION("deep depth")
            {
                {
                    auto handle = callSteps<callSteps<never>>();
                    REQUIRE(!handle());
                }
                {
                    auto handle = callSteps<callSteps<always>>();
                    handle();
                    handle();
                    REQUIRE(!handle.is_resumable());
                }
                {
                    auto handle = always2();
                    handle();
                    handle();
                    REQUIRE(!handle());
                }
                {
                    auto handle = callSteps<always, callSteps<always>, callSteps<never, always, never>>();
                    REQUIRE(n_steps(handle, 4));
                }
            }
        }
    }
}
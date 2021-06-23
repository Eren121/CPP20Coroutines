#include <catch2/catch.hpp>
#include "debug.hpp"
#include "task.hpp"

using namespace coro;

namespace
{
    task<int> noop()
    {
        co_return 10;
    }

    task<int> foo()
    {

        co_await std::suspend_always{};
        co_return 20;
    }

    task<int> bar()
    {
        int y = co_await foo() * 2;

        co_return y;
    }

    task<int> bar2()
    {
        int x = co_await bar() * 2;
        co_return x;
    }

    task<> v()
    {
        co_await foo();
    }

    TEST_CASE("task<non-void>")
    {
        {
            auto handle = v();
            REQUIRE(n_steps(handle, 2));
        }
        {
            auto handle = noop();
            noop();
            REQUIRE(n_steps(handle, 1, 10));
        }
        {
            auto handle = foo();
            REQUIRE(n_steps(handle, 2, 20));
        }
        {
            auto handle = bar();
            REQUIRE(n_steps(handle, 2, 40));
        }
        {
            auto handle = bar2();
            handle();
            handle();
            REQUIRE(!handle.is_resumable());
            REQUIRE(handle.get() == 80);
        }
    }
}
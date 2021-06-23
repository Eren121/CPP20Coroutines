#pragma once

class suspend_maybe
{
private:
    bool suspend_;

public:
    suspend_maybe(bool suspend) : suspend_(suspend) {}

    bool await_ready() const noexcept { return !suspend_; }
    constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}
    constexpr void await_resume() const noexcept {}
};
#pragma once

namespace coro
{
    template<typename T>
    bool n_steps(T& t, int n)
    {
        for (int i = 0; i < n; ++i) t();
        return !t.is_resumable();
    }

    template<typename T>
    bool n_steps(T& t, int n, auto ret)
    {
        for (int i = 0; i < n; ++i) t();
        return !t.is_resumable() && t.get() == ret;
    }
}
#pragma once

#include <coroutine>
#include <optional>
#include <cassert>
#include <memory>
#include <vector>
#include "suspend_maybe.hpp"

struct promise_type_vars
{
    using stack = std::vector<std::coroutine_handle<>>;

    std::shared_ptr<stack> stack_;
};

template<typename self>
struct promise_type_base : promise_type_vars
{
    using promise_type = typename self::promise_type;

    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }

    [[noreturn]] void unhandled_exception() { throw; }

    self get_return_object() { return {std::coroutine_handle<promise_type>::from_promise(static_cast<promise_type&>(*this))}; }
};

template<typename T>
struct add_reference
{
    using type = T&;
};

template<>
struct add_reference<void>
{
    using type = void;
};

template<typename T>
using add_reference_t = typename add_reference<T>::type;

template<typename T = void>
class task
{
    static constexpr bool is_void = std::is_same_v<T, void>;

public:

    struct promise_type_nonvoid : promise_type_base<task>
    {
        std::optional<T> t_;
        void return_value(T t)
        {
            t_ = std::move(t);
            this->stack_->pop_back();
        }
    };

    struct promise_type_void : promise_type_base<task>
    {
        void return_void()
        {
            this->stack_->pop_back();
        }
    };

    struct promise_type : std::conditional_t<is_void, promise_type_void, promise_type_nonvoid>
    {
        using stack = promise_type_vars::stack;
    };

private:
    std::coroutine_handle<promise_type> h_;

public:
    explicit task() = default;
    task(std::coroutine_handle<promise_type> h) : h_(h)
    {
        h_.promise().stack_ = std::make_shared<typename promise_type::stack>();
        h_.promise().stack_->push_back(h_);
    }

    task(const task&) = delete;
    task& operator=(const task&) = delete;

    task(task&& other)
    {
        swap(h_, other.h_);
    }

    task& operator=(task&& other)
    {
        swap(h_, other.h_);
        return *this;
    }

    ~task()
    {
        if(h_)
        {
            h_.destroy();
            h_ = {};
        }
    }

    bool is_resumable() const { return h_ && !h_.done(); }
    bool operator()() { return resume(); }

    auto& stack() const { return *h_.promise().stack_; }

    bool resume()
    {
        assert(is_resumable());

        auto& s = *h_.promise().stack_;

        auto back = s.back();

        back.resume(); //execute suspended point
        // at this point, back and s.back() could be different


        if(back.done())
        {
            if(s.size() > 0)
            {
                // Not root, execute co_await return expression
                resume();
            }
            else
            {
                if constexpr(!is_void)
                {
                    assert(h_.promise().t_ && "Should return a value to caller");
                }
            }
        }

        return is_resumable();
    }

    // Make co_await'able (allow recursion and inner task as part of the parent task) ------------------------
    bool await_ready() const
    {
        return false;
    }

    template<typename X>
    bool await_suspend(std::coroutine_handle<X>& p)
    {
        // --- stack push
        h_.promise().stack_ = p.promise().stack_;
        p.promise().stack_->push_back(h_);
        // ---

        h_(); // never wait an inner function whether initially or finally (initially managed here)

        if(!h_.done())
        {
            // the inner coroutine has at least one suspend point
            return true;
        }
        else
        {
            return false; // don't wait if the coroutine is already ended (coroutine is NOOP)
        }
    }

    T await_resume()
    {
        if constexpr(!is_void)
        {
            assert(h_.promise().t_ && "Should return a value in a co_wait expression");
            return get();
        }
    }

    add_reference_t<T> get()
    {
        if constexpr(!is_void)
        {
            return h_.promise().t_.value();
        }
    }
};

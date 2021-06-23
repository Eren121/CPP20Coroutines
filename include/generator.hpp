#pragma once

#include <coroutine>
#include <cassert>
#include <optional>
#include "suspend_maybe.hpp"

template<typename T>
class generator
{
public:
    struct promise_type
    {
        std::optional<T> t_;

        std::optional<generator> innerCoroutine_;

        promise_type() = default;
        ~promise_type() = default;

        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        [[noreturn]] void unhandled_exception() { throw; }
        generator get_return_object() { return {std::coroutine_handle<promise_type>::from_promise(*this)}; }

        std::suspend_always yield_value(T t) { t_ = std::move(t); return {}; }
        void return_void() {}

        suspend_maybe yield_value(generator innerCoroutine)
        {
            innerCoroutine_ = std::move(innerCoroutine);

            // Get first return value of the inner coroutine
            if(!innerCoroutine_->is_resumable())
            {
                // coroutine is default-constructed (why TF yielding it), cancel it and do not suspend
                innerCoroutine_ = {};
                return false;
            }
            else
            {
                // the coroutine is valid; run it to get the first yield (or until end)
                innerCoroutine_->operator()();

                if(!innerCoroutine_->is_resumable())
                {
                    // the coroutine has yielded no value and is already ended, cancel it and do not suspend
                    innerCoroutine_ = {};
                    return false;
                }
                else
                {
                    t_ = std::move(innerCoroutine_->get()); // the coroutine has yield its first value; save it
                    return true;
                }
            }
        }
    };
private:
    std::coroutine_handle<promise_type> h_;

    generator(std::coroutine_handle<promise_type> h) : h_(h) {}

public:
    explicit generator() = default;

    // ------ Prevent copies
    generator(const generator&) = delete;
    generator& operator=(const generator&) = delete;

    // ------ Allow moves
    generator(generator&& other) noexcept
    {
        std::swap(h_, other.h_);
    }

    generator& operator=(generator&& other) noexcept
    {
        std::swap(h_, other.h_);
        return *this;
    }

    ~generator()
    {
        if(h_)
        {
            h_.destroy();
            h_ = {};
        }
    }

    bool is_resumable() const
    {
        return h_ && !h_.done();
    }

    bool operator()()
    {
        return resume();
    }

    bool resume()
    {
        assert(is_resumable());

        if(h_.promise().innerCoroutine_)
        {
            // The coroutine has yielded another inner coroutine that is not finished yet
            // run it until next yield or its end

            h_.promise().innerCoroutine_->resume();
            if(h_.promise().innerCoroutine_->is_resumable())
            {
                h_.promise().t_ = std::move(h_.promise().innerCoroutine_->get()); // yielded a value - save it

                return true;
            }
            else
            {
                h_.promise().innerCoroutine_ = {};
                h_(); // inner coroutine has stopped - continue main coroutine

                return !h_.done();
            }
        }
        else
        {
            h_();

            return !h_.done();
        }
    }

    [[nodiscard]] const T& get() const
    {
        return h_.promise().t_.value();
    }

    [[nodiscard]] T& get() // Allow movable
    {
        return h_.promise().t_.value();
    }

    // ----------------- Range based loop stuff

    class iterator
    {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = value_type*;
        using reference = value_type&;

        iterator(generator* gen = nullptr) : gen_(gen)
        {
            if(gen_)
            {
                if(!gen_->is_resumable())
                {
                    gen_ = nullptr; // gen_ is an default-constructed generator
                }
                else
                {
                    ++(*this); // Since the generator does not run at creation, run until first yield (or until end)
                }
            }
        }

        auto& operator*() const { return gen_->get(); }

        bool operator==(const iterator&) const = default;

        iterator& operator++()
        {
            gen_->resume();
            if(!gen_->is_resumable())
            {
                gen_ = nullptr;
            }

            return *this;
        }

    private:

        generator* gen_; // If set to nullptr, this is an end pointer
    };

    iterator begin() { return {this}; }
    iterator end() { return {}; }
};
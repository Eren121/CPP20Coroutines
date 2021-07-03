# CPP20Coroutines
Header-Only C++20 Coroutines library

This repository aims to demonstrate the capabilities of C++20 coroutines.


# generator<T>

Generates values, recursive calls are possible.


```c++
#include <iostream>
#include <generator.hpp>

// generates the first n non-negative numbers
generator<int> first_n(int n)
{
    for(int i = 0; i < n; ++i)
    {
        co_yield i;
    }
}

```

# task<T>

Generates a single value at the end with a `co_return` expression. A `task` may `co_awaiy` another `task`.

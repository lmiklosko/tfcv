#pragma once
#include <chrono>
#include <utility>
#include <functional>
#include <type_traits>
#include <span>

namespace utility
{
    template<typename Period = std::chrono::milliseconds, typename Callable>
    Period measure_time(Callable&& callable) noexcept(std::is_nothrow_invocable_v<Callable>)
    {
        auto start = std::chrono::high_resolution_clock::now();
        std::invoke(std::forward<Callable>(callable));
        auto end = std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast<Period>(end - start);
    }

    template<typename T>
    std::vector<std::span<T>> split(std::span<T> items, size_t size)
    {
        std::vector<std::span<T>> out;
        for (size_t i = 0; i < items.size(); i += size)
        {
            out.push_back(items.subspan(i, std::min(size, items.size() - i)));
        }
        return out;
    }
}
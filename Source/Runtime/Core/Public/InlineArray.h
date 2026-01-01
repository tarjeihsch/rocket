#pragma once

#include "Assertion.h"

#include <array>
#include <span>
#include <vector>
#include <cstddef>

template<typename T, size_t N>
class TInlineArray
{
public:
    TInlineArray() = default;

    explicit TInlineArray(const std::array<T, N>& array)
    {
        assign(array);
    }

    explicit TInlineArray(const std::span<T>& array)
    {
        assign(array);
    }

    explicit TInlineArray(const std::vector<T>& array)
    {
        assign(array);
    }

    void push_back(const T& element)
    {
        ensure(count < N);
        storage[count] = element;
        count++;
    }

    void push_back(T&& element)
    {
        ensure(count < N);
        storage[count] = std::move(element);
        count++;
    }

    void assign(std::span<const T> src)
    {
        clear();

        count = (src.size() > N) ? N : src.size();

        for (size_t i = 0; i < count; ++i)
        {
            storage[i] = src[i];
        }
    }

    void append(std::span<const T> src)
    {
        for (const T& element : src)
        {
            if (count >= N)
            {
                break;
            }

            storage[count] = element;
            count++;
        }
    }

    void clear()
    {
        count = 0;
    }

    static constexpr size_t capacity() noexcept { return N; }
    size_t num() const noexcept { return count; }

    T& operator[](size_t i)
    {
        ensure(i < count);
        return storage[i];
    }

    const T& operator[](size_t i) const
    {
        ensure(i < count);
        return storage[i];
    }

    [[nodiscard]] std::span<T> as_span() noexcept
    {
        return std::span<T>(storage.data(), count);
    }

    [[nodiscard]] std::span<const T> as_span() const noexcept
    {
        return std::span<const T>(storage.data(), count);
    }

    T* begin()
    {
        return storage.data();
    }

    T* end()
    {
        return storage.data() + count;
    }

    const T* begin() const
    {
        return storage.data();
    }

    const T* end() const
    {
        return storage.data() + count;
    }

private:
    std::array<T, N> storage = {};
    size_t count = {};
};

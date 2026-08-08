#pragma once

#include <assert.h>

template<typename T, size_t Capacity>
class StaticArray
{
public:
    StaticArray()
    {
        if constexpr (std::is_trivial_v<T>)
        {
            std::fill_n(data, Capacity, T{});
        }
    }

    void Push(T value)
    {
        assert(count < Capacity);
        data[count++] = value;
    }

    void Insert(const void* src, size_t size)
    {
        assert(count + size <= Capacity);
        memcpy(data + count, src, size);
        count += size;
    }

    void Pop()
    {
        static_assert(count > 0);
        
        if constexpr (std::is_trivial_v<T>)
        {
            data[count] = T{};
        }
    }

    void Clear()
    {
        count = 0;
    }

    [[nodiscard]] 
    inline size_t Size() const 
    { 
        return count; 
    }
    
    [[nodiscard]] 
    inline constexpr size_t GetCapacity() const 
    { 
        return Capacity; 
    }

    inline T& operator[](size_t index)
    {
        assert(index < count);
        return data[index];
    }

    inline const T& operator[](size_t index) const
    {
        assert(index < count);
        return data[index];
    }

    T* Data() 
    {
        return data;
    }

private:
    T data[Capacity] = {};
    size_t count = 0;
};
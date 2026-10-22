#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

#include "CoreTypes.h"

// Delegates are declared with the signature they are called with:
//
//     Delegate<void(Vector2i)> on_resize;
//     MulticastDelegate<void(Vector2i)> on_resized;
//
// Both are backed by std::function for the time being, so anything std::function
// accepts can be bound.

template<typename Signature>
class Delegate;

// A single callable. Binding replaces the callable that was bound before.
template<typename ReturnType, typename... Args>
class Delegate<ReturnType(Args...)>
{
public:
    Delegate() = default;

    Delegate(std::nullptr_t)
    {
    }

    template<typename Callable>
        requires (!std::is_same_v<std::decay_t<Callable>, Delegate> && std::is_invocable_r_v<ReturnType, Callable&, Args...>)
    Delegate(Callable&& callable)
        : function(std::forward<Callable>(callable))
    {
    }

    template<typename Callable>
    void Bind(Callable&& callable)
    {
        function = std::forward<Callable>(callable);
    }

    void Unbind()
    {
        function = nullptr;
    }

    bool IsBound() const
    {
        return static_cast<bool>(function);
    }

    explicit operator bool() const
    {
        return IsBound();
    }

    // Calling a delegate that is not bound is an error, so check IsBound() first.
    ReturnType operator()(Args... args) const
    {
        return function(std::forward<Args>(args)...);
    }

private:
    std::function<ReturnType(Args...)> function;
};

// Identifies one callable added to a MulticastDelegate, so that it can be removed.
struct DelegateHandle
{
    u64 id = 0;

    bool IsValid() const
    {
        return id != 0;
    }

    bool operator==(const DelegateHandle&) const = default;
};

template<typename Signature>
class MulticastDelegate;

// Any number of callables called in the order they were added. A callable has no
// result to give back, so the signature returns void.
template<typename... Args>
class MulticastDelegate<void(Args...)>
{
public:
    DelegateHandle Add(Delegate<void(Args...)> delegate)
    {
        const DelegateHandle handle = { ++last_id };

        entries.push_back({ handle, std::move(delegate) });

        return handle;
    }

    // Returns whether the handle was still bound.
    bool Remove(DelegateHandle handle)
    {
        const auto entry = std::find_if(entries.begin(), entries.end(), [handle](const Entry& candidate)
        {
            return candidate.handle == handle;
        });

        if (entry == entries.end())
        {
            return false;
        }

        entries.erase(entry);

        return true;
    }

    void Clear()
    {
        entries.clear();
    }

    bool IsBound() const
    {
        return !entries.empty();
    }

    // A callable may add or remove callables, including itself. The ones added
    // during a broadcast are first called by the next one, and one that was
    // removed before its turn is not called, as whatever it captured may be gone.
    void Broadcast(Args... args) const
    {
        const std::vector<Entry> snapshot = entries;

        for (const Entry& entry : snapshot)
        {
            const bool removed = std::none_of(entries.begin(), entries.end(), [&entry](const Entry& candidate)
            {
                return candidate.handle == entry.handle;
            });

            if (!removed)
            {
                entry.delegate(args...);
            }
        }
    }

    void operator()(Args... args) const
    {
        Broadcast(args...);
    }

private:
    struct Entry
    {
        DelegateHandle handle;
        Delegate<void(Args...)> delegate;
    };

    std::vector<Entry> entries;
    u64 last_id = 0;
};

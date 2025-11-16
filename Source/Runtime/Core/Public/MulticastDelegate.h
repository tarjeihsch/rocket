#pragma once

#include <vector>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <cstddef>

// Signature form: TSmallFunction<R(Args...), InlineSize, InlineAlign>
template<typename Signature, std::size_t InlineSize = 32, std::size_t InlineAlign = alignof(std::max_align_t)>
class TSmallFunction;

template<typename R, typename... Args, std::size_t InlineSize, std::size_t InlineAlign>
class TSmallFunction<R(Args...), InlineSize, InlineAlign>
{
public:
    TSmallFunction() = default;

    TSmallFunction(std::nullptr_t)
    {
    }

    TSmallFunction(const TSmallFunction& Other)
    {
        if (Other.HasValue())
        {
            Other.VTable.Copy(Storage(), Other.Storage());
            VTable = Other.VTable;
            bHasValue = true;
        }
    }

    TSmallFunction(TSmallFunction&& Other) noexcept
    {
        if (Other.HasValue())
        {
            Other.VTable.Move(Storage(), Other.Storage());
            VTable = Other.VTable;
            bHasValue = true;
            Other.Reset();
        }
    }

    template<typename F,
             typename = std::enable_if_t<!std::is_same<std::decay_t<F>, TSmallFunction>::value>>
    TSmallFunction(F&& Func)
    {
        Bind(std::forward<F>(Func));
    }

    ~TSmallFunction()
    {
        Reset();
    }

    TSmallFunction& operator=(const TSmallFunction& Other)
    {
        if (this != &Other)
        {
            Reset();
            if (Other.HasValue())
            {
                Other.VTable.Copy(Storage(), Other.Storage());
                VTable = Other.VTable;
                bHasValue = true;
            }
        }
        return *this;
    }

    TSmallFunction& operator=(TSmallFunction&& Other) noexcept
    {
        if (this != &Other)
        {
            Reset();
            if (Other.HasValue())
            {
                Other.VTable.Move(Storage(), Other.Storage());
                VTable = Other.VTable;
                bHasValue = true;
                Other.Reset();
            }
        }
        return *this;
    }

    TSmallFunction& operator=(std::nullptr_t)
    {
        Reset();
        return *this;
    }

    template<typename F>
    void Bind(F&& Func)
    {
        using Functor = std::decay_t<F>;

        static_assert(sizeof(Functor) <= InlineSize,
                      "TSmallFunction: functor too large for inline storage");
        static_assert(alignof(Functor) <= InlineAlign,
                      "TSmallFunction: functor alignment too strict for inline storage");
        static_assert(std::is_move_constructible<Functor>::value,
                      "TSmallFunction: functor must be move constructible");

        Reset();

        new (Storage()) Functor(std::forward<F>(Func));

        VTable.Invoke = &InvokeImpl<Functor>;
        VTable.Copy   = &CopyImpl<Functor>;
        VTable.Move   = &MoveImpl<Functor>;
        VTable.Destroy= &DestroyImpl<Functor>;

        bHasValue = true;
    }

    void Reset()
    {
        if (bHasValue)
        {
            VTable.Destroy(Storage());
            bHasValue = false;
            VTable = {};
        }
    }

    bool HasValue() const
    {
        return bHasValue;
    }

    explicit operator bool() const
    {
        return HasValue();
    }

    R operator()(Args... args) const
    {
        // const-call; storage pointer is still non-const because we may store
        // non-const functors, but Invoke must not modify them unless callable is mutable.
        return VTable.Invoke(const_cast<void*>(static_cast<const void*>(Storage())),
                             std::forward<Args>(args)...);
    }

private:
    struct FVTable
    {
        using InvokeFn  = R(*)(void*, Args&&...);
        using CopyFn    = void(*)(void* dst, const void* src);
        using MoveFn    = void(*)(void* dst, void* src);
        using DestroyFn = void(*)(void* obj);

        InvokeFn  Invoke  = nullptr;
        CopyFn    Copy    = nullptr;
        MoveFn    Move    = nullptr;
        DestroyFn Destroy = nullptr;
    };

    template<typename Functor>
    static R InvokeImpl(void* Obj, Args&&... args)
    {
        Functor* F = reinterpret_cast<Functor*>(Obj);
        if constexpr (std::is_void_v<R>)
        {
            (*F)(std::forward<Args>(args)...);
        }
        else
        {
            return (*F)(std::forward<Args>(args)...);
        }
    }

    template<typename Functor>
    static void CopyImpl(void* Dst, const void* Src)
    {
        const Functor* SrcF = reinterpret_cast<const Functor*>(Src);
        new (Dst) Functor(*SrcF);
    }

    template<typename Functor>
    static void MoveImpl(void* Dst, void* Src)
    {
        Functor* SrcF = reinterpret_cast<Functor*>(Src);
        new (Dst) Functor(std::move(*SrcF));
        SrcF->~Functor();
    }

    template<typename Functor>
    static void DestroyImpl(void* Obj)
    {
        Functor* F = reinterpret_cast<Functor*>(Obj);
        F->~Functor();
    }

    void* Storage()
    {
        return static_cast<void*>(Buffer);
    }

    const void* Storage() const
    {
        return static_cast<const void*>(Buffer);
    }

    alignas(InlineAlign) unsigned char Buffer[InlineSize] = {};
    FVTable VTable = {};
    bool bHasValue = false;
};

// Include TSmallFunction header above
// #include "SmallFunction.h"

struct FDelegateHandle
{
    uint64_t Id = 0;

    FDelegateHandle() = default;
    explicit FDelegateHandle(uint64_t InId) : Id(InId) {}

    bool IsValid() const { return Id != 0; }

    friend bool operator==(const FDelegateHandle& A, const FDelegateHandle& B)
    {
        return A.Id == B.Id;
    }

    friend bool operator!=(const FDelegateHandle& A, const FDelegateHandle& B)
    {
        return !(A == B);
    }
};

template<typename... Args>
class TMulticastDelegate
{
public:
    // You can tweak the inline size here depending on your typical lambdas
    using FuncType = TSmallFunction<void(Args...), 32>;

    TMulticastDelegate() = default;

    FDelegateHandle Add(FuncType Func)
    {
        const FDelegateHandle Handle(NextId++);
        Entries.push_back(FEntry{Handle, std::move(Func)});
        return Handle;
    }

    template<typename F>
    FDelegateHandle AddLambda(F&& Func)
    {
        return Add(FuncType(std::forward<F>(Func)));
    }

    template<typename T>
    FDelegateHandle AddMember(T* Obj, void (T::*Method)(Args...))
    {
        return AddLambda([Obj, Method](Args... args)
        {
            (Obj->*Method)(std::forward<Args>(args)...);
        });
    }

    template<typename T>
    FDelegateHandle AddMember(const T* Obj, void (T::*Method)(Args...) const)
    {
        return AddLambda([Obj, Method](Args... args)
        {
            (Obj->*Method)(std::forward<Args>(args)...);
        });
    }

    void Remove(FDelegateHandle Handle)
    {
        if (!Handle.IsValid())
            return;

        if (BroadcastDepth > 0)
        {
            for (FEntry& Entry : Entries)
            {
                if (Entry.Handle == Handle)
                {
                    Entry.bMarkedForRemove = true;
                    break;
                }
            }
        }
        else
        {
            EraseByHandle(Handle);
        }
    }

    void Clear()
    {
        if (BroadcastDepth > 0)
        {
            for (FEntry& Entry : Entries)
            {
                Entry.bMarkedForRemove = true;
            }
        }
        else
        {
            Entries.clear();
        }
    }

    bool IsBound() const
    {
        for (const FEntry& Entry : Entries)
        {
            if (!Entry.bMarkedForRemove && Entry.Func)
                return true;
        }
        return false;
    }

    void Broadcast(Args... args)
    {
        ++BroadcastDepth;

        for (const FEntry& Entry : Entries)
        {
            if (!Entry.bMarkedForRemove && Entry.Func)
            {
                Entry.Func(std::forward<Args>(args)...);
            }
        }

        --BroadcastDepth;

        if (BroadcastDepth == 0)
        {
            Compact();
        }
    }

private:
    struct FEntry
    {
        FDelegateHandle Handle;
        FuncType        Func;
        bool            bMarkedForRemove = false;
    };

    std::vector<FEntry> Entries;
    uint64_t            NextId         = 1;
    int32_t             BroadcastDepth = 0;

    void EraseByHandle(FDelegateHandle Handle)
    {
        auto It = Entries.begin();
        while (It != Entries.end())
        {
            if (It->Handle == Handle)
            {
                It = Entries.erase(It);
                break;
            }
            else
            {
                ++It;
            }
        }
    }

    void Compact()
    {
        auto It = Entries.begin();
        while (It != Entries.end())
        {
            if (It->bMarkedForRemove)
            {
                It = Entries.erase(It);
            }
            else
            {
                ++It;
            }
        }
    }
};
#pragma once

#include <cstdint>

class RObject;
class RActor;

/*
 * RClass is a static-initialized runtime metadata layer for its outer RObject archetype.
 *
 * Each Object is responsible for self-declaring its own class reflection metadata using the macro CLASS() from within object body.
 * This approach guarantees fast type-safety lookup and comparisons, while also ensuring that each object owns their reflection data and properties.
 * Objects are constructed and destructed using proc addresses, to ensure that the derived type size is correct.
 * TODO: Consider using a uint32_t Type with bit masking to store Type, Flags, ID.
 */

static constexpr uint32_t C_None = 0 << 0;
static constexpr uint32_t C_ClassDefault = 1 << 0;
static constexpr uint32_t C_Memory_Allocator_Frame = 1 << 2;
static constexpr uint32_t C_Memory_Allocator_Heap = 1 << 3;

class RClass final
{
public:
    RClass() = delete;
    RClass(RClass* P_ParentClass, RObject* P_Archetype, uint64_t P_Size, uint64_t P_Alignment, uint32_t P_Flags);

    bool IsA(const RClass* Other) const;

    inline const RClass* GetSuperClass() const
    {
        return SuperClass;
    }

    inline const RObject* GetArchetype() const
    {
        return Archetype;
    }

    inline uint64_t GetSize() const
    {
        return Size;
    }

    inline uint64_t GetAlignment() const
    {
        return Alignment;
    }

    inline uint32_t GetFlags() const
    {
        return Flags;
    }

protected:
    // Live pointer to parents static class storage - allows walking up the inheritance hierarchy
    RClass* SuperClass;

    // Object archetype - copies values directly onto newly constructed instances
    RObject* Archetype;

    // Object size in bytes
    uint64_t Size;

    // Object alignment in bytes
    uint64_t Alignment;

    // Class flags
    uint32_t Flags;

#if RK_OBJECT_CUSTOM_ALLOCATOR
    using FConstructFn = RObject*(*)();

    // Allocate object function callback
    FConstructFn NewFn;
#endif
};
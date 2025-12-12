#pragma once

#include "glm/vec3.hpp"

#include <concepts>
#include <cstdint>
#include <span>
#include <cstring>

class RObject;
class RActor;
class RClass;

enum class EFieldType : uint8_t
{
    Bool,
    Int,
    Float,
    Double,
    Vector,
    Object
};

class RField final
{
public:
    const char* Name;
    size_t Offset;
    size_t Size;
    size_t Alignment;
    RClass* FieldClass;
    EFieldType FieldType;

    void SetValue(RObject* Object, const void* Value) const
    {
        uint8_t* Bytes = reinterpret_cast<uint8_t*>(Object);
        std::memcpy(Bytes + Offset, Value, Size);
    }

    void* GetValue(RObject* Object) const
    {
        uint8_t* Bytes = reinterpret_cast<uint8_t*>(Object);
        return Bytes + Offset;
    }
};

template<typename T>
struct RTypeTag;

template<>
struct RTypeTag<bool> { static constexpr EFieldType Value = EFieldType::Bool; };

template<>
struct RTypeTag<int> { static constexpr EFieldType Value = EFieldType::Int; };

template<>
struct RTypeTag<float> { static constexpr EFieldType Value = EFieldType::Float; };

template<>
struct RTypeTag<double> { static constexpr EFieldType Value = EFieldType::Double; };

template<>
struct RTypeTag<glm::vec3> { static constexpr EFieldType Value = EFieldType::Vector; };

template<>
struct RTypeTag<RObject*> { static constexpr EFieldType Value = EFieldType::Object; };

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
    RClass(const char* ClassName, RClass* P_ParentClass, RObject* P_Archetype, size_t P_Size, size_t P_Alignment, uint32_t P_Flags, RField* Fields = nullptr, size_t FieldCount = 0);

    bool IsA(const RClass* Other) const;

    inline const char* GetClassName() const
    {
        return ClassName;
    }

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

    inline std::span<RField> GetFields() const
    {
        return std::span<RField>(Fields, FieldCount);
    }

protected:
    const char* ClassName;

    // Live pointer to parents static class storage - allows walking up the inheritance hierarchy
    RClass* SuperClass;

    // Object archetype - copies values directly onto newly constructed instances
    RObject* Archetype;

	// Array of reflected fields of this class
    RField* Fields;

	// Number of reflected fields of this class
	size_t FieldCount;

    // Object size in bytes
    size_t Size;

    // Object alignment in bytes
    size_t Alignment;

    // Class flags
    uint32_t Flags;

#if RK_OBJECT_CUSTOM_ALLOCATOR
    using FConstructFn = RObject*(*)();

    // Allocate object function callback
    FConstructFn NewFn;
#endif
};

template <typename T>
concept DoesImplementStaticClass = requires()
{
    { T::StaticClass() } -> std::same_as<RClass*>;
};

#define RK_FIELD(Type, Class, Member) \
    { \
        #Member, \
        offsetof(Class, Member), \
        sizeof(((Class*)0)->Member), \
        alignof(Type), \
        nullptr, \
        RTypeTag<Type>::Value \
    }


#pragma once

#include "Assertion.h"
#include "Class.h"
#include "UUID64.h"

#define PURE_VIRTUAL { }

class RWorld;

/*
 * Each RObject carries its own integrated RClass metadata layer. This results in boilerplate code that is necessary to perform
 * statical reflection-based lookup and comparisons.
 *
 * Heap-allocated data initialized within the CTOR is not guaranteed to be deep-copied or unique.
 * Use CTOR to initialize trivial data types or structures that are to be shared between instances.
 *
 * Trivially copyable value types such as 'int' and 'float' have copy semantics that work by value.
 * Other complex data types should do their initialization in PostConstruct(). Copy CTOR's are disabled by design.
 *
 */
class RObject
{
public:
    RObject();
    RObject(const RObject&) = delete;
    RObject& operator=(const RObject&) = delete;
    RObject(RObject&&) = delete;
    RObject& operator=(RObject&&) = delete;

    virtual ~RObject() = default;

    static RClass* StaticClass()
    {
        static RObject ClassDefaultObject;

        static RClass Class { "RObject", nullptr, &ClassDefaultObject, sizeof(RObject), alignof(RObject), C_None };

        return &Class;
    }

    virtual const RClass* GetClass() const;

    // Runs a deep copy on pointer/heap-owning fields
    virtual void PostConstruct(RWorld* InWorld, RObject* InOuter);
    virtual void Load(const std::vector<uint8_t>& Data) PURE_VIRTUAL;
    virtual void Save(std::vector<uint8_t>& Data) PURE_VIRTUAL;
    virtual void OnPropertyChanged() PURE_VIRTUAL;

    RWorld* GetWorld() const;
    RObject* GetOuter() const;
    uint64_t GetUUID() const;

protected:
    static_assert(DoesImplementStaticClass<RObject>, "");

protected:
    RWorld* World;

    RObject* Outer;

    RUUID64 UUID;
};

#define RCLASS()
#define RFUNCTION()
#define RFIELD()

#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

struct FConfigField;
struct FConfigFieldHash;
struct FConfigSection;
struct FConfigSectionHash;

using FConfigFieldSet = std::unordered_set<FConfigField, FConfigFieldHash>;
using FConfigSectionSet = std::unordered_set<FConfigSection, FConfigSectionHash>;
using FConfigValueType = std::variant<std::monostate, bool, int32_t, double, std::string>;

struct FConfigField
{
    std::string M_Key;

    FConfigValueType M_Value;

    std::optional<uint16_t> M_Position;

    bool operator==(const FConfigField& Other) const noexcept
    {
        return M_Key == Other.M_Key;
    }
};

struct FConfigFieldHash
{
    std::size_t operator()(const FConfigField& P_Field) const noexcept
    {
        return std::hash<std::string>{}(P_Field.M_Key);
    }
};

struct FConfigSection
{
    std::string M_Name;

    mutable FConfigFieldSet M_FieldSet;

    std::optional<uint16_t> M_Position;

    bool operator==(const FConfigSection& Other) const noexcept
    {
        return M_Name == Other.M_Name;
    }

    std::string ToString() const noexcept
    {
        return "[" + M_Name + "]\n";
    }
};

struct FConfigSectionHash
{
    std::size_t operator()(const FConfigSection& P_Section) const noexcept
    {
        return std::hash<std::string>{}(P_Section.M_Name);
    }
};

enum class EConfigFileFlags : uint8_t
{
    None = 0,
    Flush = 1 << 1,
};

inline EConfigFileFlags operator|(EConfigFileFlags Lhs, EConfigFileFlags Rhs) { return static_cast<EConfigFileFlags>(static_cast<uint8_t>(Lhs) | static_cast<uint8_t>(Rhs)); }
inline EConfigFileFlags operator&(EConfigFileFlags Lhs, EConfigFileFlags Rhs) { return static_cast<EConfigFileFlags>(static_cast<uint8_t>(Lhs) & static_cast<uint8_t>(Rhs)); }

class FConfigFile
{
public:
    explicit FConfigFile(std::string P_Path, EConfigFileFlags P_Flags = EConfigFileFlags::None);
    ~FConfigFile();

    void Write();
    void Read();

    template<typename T>
    T GetData(const std::string& Section, const std::string& Key) const;

    template<typename T>
    void SetData(const std::string& Section, const std::string& Key, const T& Value);

private:
    const FConfigSection& GetSection_Internal(const std::string& Section) const;
    const FConfigField& GetField_Internal(const std::string& Section, const std::string& Field) const;

    const FConfigValueType& GetValue_Internal(const std::string& Section, const std::string& Field) const;
    void SetValue_Internal(const std::string& Section, const std::string& Field, FConfigValueType Value);

    void Init();

private:
    std::filesystem::path M_FilePath;

    FConfigSectionSet M_SectionSet;

    EConfigFileFlags M_Flags;
};

template <typename TValue>
TValue FConfigFile::GetData(const std::string &Section, const std::string &Key) const
{
    return std::get<TValue>(GetValue_Internal(Section, Key));
}

template <typename T>
void FConfigFile::SetData(const std::string &Section, const std::string &Key, const T &Value)
{
    SetValue_Internal(Section, Key, FConfigValueType(Value));
}

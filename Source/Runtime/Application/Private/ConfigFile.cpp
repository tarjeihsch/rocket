#include "ConfigFile.h"

#include <cstring>
#include <fstream>
#include <utility>

#include "Assertion.h"

namespace PrivateUtils
{
    FConfigValueType ParseFieldValue(std::string_view Raw)
    {
        std::from_chars_result Result = {};

        int32_t IntValue = {};
        Result = std::from_chars(Raw.data(), Raw.data() + Raw.size(), IntValue);
        if (Result.ec == std::errc{} && Result.ptr == Raw.data() + Raw.size())
        {
            return IntValue;
        }

        double DoubleValue = {};
        Result = std::from_chars(Raw.data(), Raw.data() + Raw.size(), DoubleValue);
        if (Result.ec == std::errc{} && Result.ptr == Raw.data() + Raw.size())
        {
            return DoubleValue;
        }

        return std::string(Raw);
    }
}

FConfigFile::FConfigFile(std::string P_Path, EConfigFileFlags P_Flags)
    : M_FilePath(P_Path), M_Flags(P_Flags)
{
    Init();
    Read();
}

FConfigFile::~FConfigFile()
{
    if ((M_Flags & EConfigFileFlags::Flush) == EConfigFileFlags::Flush)
    {
        Write();
    }
}

void FConfigFile::Init()
{
    if (M_FilePath.has_parent_path())
    {
        std::filesystem::create_directories(M_FilePath.parent_path());
    }

    if (!std::filesystem::exists(M_FilePath))
    {
        std::ofstream File(M_FilePath, std::ios::binary);
    }
}

void FConfigFile::Read()
{
    std::ifstream File(M_FilePath, std::ios::binary | std::ios::ate);
    uint32_t FileSize = File.tellg();

    std::string FileBuffer;
    FileBuffer.resize(FileSize);

    File.seekg(0, std::ios::beg);
    File.read(FileBuffer.data(), FileSize);

    M_SectionSet.clear();
    M_SectionSet.reserve(FileSize / 16);

    std::string_view Buffer(FileBuffer.data(), FileSize);
    size_t CursorPosition = 0;

    std::string SectionName;
    std::string FieldKey;
    std::string FieldValue;

    while (CursorPosition < Buffer.size())
    {
        size_t End = Buffer.find('\n', CursorPosition);
        if (End == std::string_view::npos)
        {
            End = Buffer.size();
        }

        std::string_view Line = Buffer.substr(CursorPosition, End - CursorPosition);

        if (Line.empty() || Line.front() == ';' || Line.front() == '#')
        {
            CursorPosition = End + 1;
            continue;
        }

        if (Line.front() == '[' && Line.back() == ']')
        {
            SectionName = Line.substr(1, Line.size() - 2);
        }

        size_t SeparatorPosition = Line.find('=');
        if (SeparatorPosition != std::string::npos)
        {
            FieldKey = Line.substr(0, SeparatorPosition);
            FieldValue = Line.substr(SeparatorPosition + 1, Line.size() - SeparatorPosition - 1);

            FConfigValueType Value = PrivateUtils::ParseFieldValue(FieldValue);
            SetValue_Internal(SectionName, FieldKey, Value);
        }

        CursorPosition = End + 1;
    }
}

void FConfigFile::Write()
{
    std::string Buffer;
    Buffer.reserve(4096);

    for (const auto& Section : M_SectionSet)
    {
        Buffer.append(Section.ToString());

        for (const auto& Field : Section.M_FieldSet)
        {
            Buffer.append(Field.M_Key);
            Buffer.push_back('=');

            std::visit([&Buffer](auto&& Arg)
            {
                if constexpr (std::is_same_v<std::decay_t<decltype(Arg)>, int32_t>)
                {
                    char Temp[32];
                    auto [Ptr, ErrorCode] = std::to_chars(std::begin(Temp), std::end(Temp), Arg);
                    Buffer.append(Temp, Ptr);
                }

                if constexpr (std::is_same_v<std::decay_t<decltype(Arg)>, double>)
                {
                    char Temp[64];
                    auto [Ptr, ErrorCode] = std::to_chars(std::begin(Temp), std::end(Temp), Arg, std::chars_format::general);
                    Buffer.append(Temp, Ptr);
                }

                if constexpr (std::is_same_v<std::decay_t<decltype(Arg)>, std::string>)
                {
                    Buffer.append(Arg);
                }
            }, Field.M_Value);

            Buffer.push_back('\n');
        }
    }

    std::ofstream File(M_FilePath, std::ios::binary | std::ios::trunc);
    File.write(Buffer.data(), static_cast<std::streamsize>(Buffer.size()));
    File.close();
}

const FConfigSection &FConfigFile::GetSection_Internal(const std::string &Section) const
{
    FConfigSection SectionTemp = {Section};
    auto SectionIt = M_SectionSet.find(SectionTemp);
    ensure(SectionIt != M_SectionSet.end(), "Failed to find section with given parameter.");
    return *SectionIt;
}

const FConfigField &FConfigFile::GetField_Internal(const std::string& Section, const std::string &Field) const
{
    FConfigSection SectionTemp = {Section};
    auto SectionIt = M_SectionSet.find(SectionTemp);
    ensure(SectionIt != M_SectionSet.end(), "Failed to find section with given parameter.");

    FConfigField FieldTemp = {Field};
    auto FieldIt = SectionIt->M_FieldSet.find(FieldTemp);
    ensure(FieldIt != SectionIt->M_FieldSet.end(), "Failed to find field with given parameter.");
    return *FieldIt;
}

const FConfigValueType& FConfigFile::GetValue_Internal(const std::string &Section, const std::string &Key) const
{
    FConfigSection SectionTemp = {Section};
    auto SectionIt = M_SectionSet.find(SectionTemp);
    ensure(SectionIt != M_SectionSet.end(), "Failed to find section with given parameter.");

    FConfigField FieldTemp = {Key};
    auto FieldIt = SectionIt->M_FieldSet.find(FieldTemp);
    ensure(FieldIt != SectionIt->M_FieldSet.end(), "Failed to find field with given parameter.");

    const FConfigValueType& Value = FieldIt->M_Value;
    return Value;
}

void FConfigFile::SetValue_Internal(const std::string& Section, const std::string& Field, FConfigValueType Value)
{
    FConfigSection SectionTemp = {Section};
    auto [SectionIt, Inserted] = M_SectionSet.insert(SectionTemp);

    FConfigField FieldTemp = {Field, std::move(Value)};
    auto FieldIt = SectionIt->M_FieldSet.find(FieldTemp);
    if (FieldIt != SectionIt->M_FieldSet.end())
    {
        SectionIt->M_FieldSet.erase(FieldIt);
    }
    SectionIt->M_FieldSet.insert(std::move(FieldTemp));
}

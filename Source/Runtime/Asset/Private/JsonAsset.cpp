#include "JsonAsset.h"

#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

FJsonAsset ConvertFromRapidJson(const rapidjson::Value& val)
{
    FJsonAsset asset;

    if (val.IsNull())
    {
        asset.Value = nullptr;
    }
    else if (val.IsBool())
    {
        asset.Value = val.GetBool();
    }
    else if (val.IsNumber())
    {
        asset.Value = val.GetDouble(); // double covers int/float as well
    }
    else if (val.IsString())
    {
        asset.Value = std::string(val.GetString(), val.GetStringLength());
    }
    else if (val.IsArray())
    {
        FJsonAsset::Array arr;
        arr.reserve(val.Size());
        for (auto& v : val.GetArray())
        {
            arr.push_back(ConvertFromRapidJson(v));
        }
        asset.Value = std::move(arr);
    }
    else if (val.IsObject())
    {
        FJsonAsset::Object obj;
        for (auto it = val.MemberBegin(); it != val.MemberEnd(); ++it)
        {
            obj.emplace(
                std::string(it->name.GetString(), it->name.GetStringLength()),
                ConvertFromRapidJson(it->value)
            );
        }
        asset.Value = std::move(obj);
    }

    return asset;
}

FJsonAsset TAssetLoader<FJsonAsset, FJsonAssetOptions>::Load(const FAssetLoaderRequest &P_Request, const FJsonAssetOptions &P_Options)
{
    FJsonAsset Asset;

    std::string FileBuffer;
    std::ifstream file(P_Request.Path);
    FileBuffer.assign((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));

    rapidjson::Document Doc;
    Doc.Parse(FileBuffer.c_str());

    return ConvertFromRapidJson(Doc);
}

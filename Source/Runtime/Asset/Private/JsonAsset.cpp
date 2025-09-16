#include "JsonAsset.h"

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

FJsonAsset TAssetLoader<FJsonAsset, FJsonAssetOptions>::Load(const FAssetLoaderRequest &P_Request, const FJsonAssetOptions &P_Options)
{
    FJsonAsset Asset;

    const char* FileBuffer = "";

    rapidjson::Document Doc;
    Doc.Parse(FileBuffer);

    return Asset;
}

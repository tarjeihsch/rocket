#pragma once

#include <vector>
#include <assert.h>
#include <fstream>

inline std::vector<char> LoadFile(const char* path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    assert(file);

    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(file_size);
    file.read(buffer.data(), file_size);

    return buffer;
}

inline void SaveFile(const char* path, const std::vector<uint8_t>& data)
{
    std::ofstream file(path, std::ios::binary);
    assert(file);

    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

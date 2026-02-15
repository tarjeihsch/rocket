#pragma once

#include <vector>
#include <assert.h>

static std::vector<char> LoadFile(const char* path)
{
    FILE* file = fopen(path, "rb");
    assert(file);

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    std::vector<char> buffer(size);
    fread(buffer.data(), 1, size, file);
    fclose(file);

    return buffer;
}
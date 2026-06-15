#pragma once

#include <Math/Vector.h>

struct Vertex
{
    Vector3 position;
};

struct Face 
{
    uint32_t vertex_data[3];
};

struct Bounds
{
    Vector3 min;
    Vector3 max;
};

struct Surface 
{
    Vertex* vertex_data;
    Face* face_data;

    uint32_t vertex_count;
    uint32_t face_count;
};

float ComputeSurfaceArea(const Surface& surface);
float ComputeSurfaceVolume(const Surface& surface);
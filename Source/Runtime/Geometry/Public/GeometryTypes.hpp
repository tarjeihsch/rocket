#pragma once

#include <vector>

#include <Math/Vector.h>
#include <Color.h>

struct Vertex
{
    Vector3 position;
    Vector3 normal;
    Color32 color;
};

struct Face
{
    uint32_t index_data[3];
};

struct AABB
{
    Vector3 min;
    Vector3 max;
};

struct Ray
{
    // Origin point
    Vector3 origin;

    // Direction vector
    Vector3 direction;
};

struct RayHit
{
    RayHit()
        : hit(false)
        , distance(FLT_MAX)
        , position()
        , normal()
        , face(nullptr)
    {
    }

    // Is this ray hit valid?
    bool hit;
    
    // Distance traversed by ray
    float distance;

    // Point of intersection with face
    Vector3f position;

    // Surface normal at the hit point
    Vector3f normal;

    // ID of face
    const Face* face;
};

struct Plane
{
    // Outward normal
    Vector3 normal;

    // Signed distance from the world origin to the plane
    float distance;
};

/**
 * CPU-side triangle mesh data.
 *
 * The same mesh representation is used for models and transient UI geometry.
 * UI vertices store their texture coordinates in the normal.xyz slot so their
 * byte layout remains identical to model vertices.
 */
struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<Face> faces;

    AABB aabb;
};

struct Frustum
{
    // Left, right, top, bottom, near, far
    Plane planes[6];
};

struct RayQuery
{
    Ray ray;
    const uint32_t* ignored_actors;
    uint32_t ignored_actor_count;
    float max_distance;
    uint32_t collision_channels;
};

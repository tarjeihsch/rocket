#include "GeometryUtils.hpp"

AABB ComputeSurfaceAABB(const Mesh &surface)
{
    AABB result = {};

    if (surface.vertices.empty())
    {
        return result;
    }

    result.min = surface.vertices[0].position;
    result.max = surface.vertices[0].position;

    for (uint32_t index = 0; index < surface.vertices.size(); ++index)
    {
        const Vertex& vertex = surface.vertices[index];
        
        result.min.x = Math::Min(result.min.x, vertex.position.x);
        result.min.y = Math::Min(result.min.y, vertex.position.y);
        result.min.z = Math::Min(result.min.z, vertex.position.z);

        result.max.x = Math::Max(result.max.x, vertex.position.x);
        result.max.y = Math::Max(result.max.y, vertex.position.y);
        result.max.z = Math::Max(result.max.z, vertex.position.z);
    }

    return result;
}

float ComputeSurfaceArea(const Mesh &surface)
{
    return 0.0f;
}

float ComputeSurfaceVolume(const Mesh &surface)
{
    return 0.0f;
}

bool ContainsAABB(const AABB& self, const AABB& other)
{
    return self.min.x <= other.min.x && self.min.y <= other.min.y &&
           self.min.z <= other.min.z && self.max.x >= other.max.x &&
           self.max.y >= other.max.y && self.max.z >= other.max.z;
}

bool IntersectsAABB(const AABB& self, const AABB& other)
{
    return self.min.x <= other.max.x && self.max.x >= other.min.x &&
           self.min.y <= other.max.y && self.max.y >= other.min.y &&
           self.min.z <= other.max.z && self.max.z >= other.min.z;
}

static Plane MakeFrustumPlane(float a, float b, float c, float d)
{
    Plane plane;

    Vector3 normal = Vector3(a, b, c);
    const float length = normal.Length();

    if (length > 1e-8f)
    {
        const float inverse_length = 1.0f / length;
        normal *= inverse_length;
        d *= inverse_length;
    }

    plane.normal = normal;
    plane.distance = d;

    return plane;
}

Frustum ComputeFrustum(const Matrix4x4& view_matrix, const Matrix4x4& projection_matrix)
{
    // Column-major matrix multiplication, matches usage in ConstructRayFromMouse
    const Matrix4x4 combined = projection_matrix * view_matrix;

    // Row i of the combined matrix, expressed as plane coefficients (a, b, c, d)
    // combined.Data[Column][Row] stores the element at (Row, Column)
    const float row0[4] = { combined.Data[0][0], combined.Data[1][0], combined.Data[2][0], combined.Data[3][0] };
    const float row1[4] = { combined.Data[0][1], combined.Data[1][1], combined.Data[2][1], combined.Data[3][1] };
    const float row2[4] = { combined.Data[0][2], combined.Data[1][2], combined.Data[2][2], combined.Data[3][2] };
    const float row3[4] = { combined.Data[0][3], combined.Data[1][3], combined.Data[2][3], combined.Data[3][3] };

    Frustum frustum;

    // Left, right, top, bottom, near, far - normals point towards frustum interior
    frustum.planes[0] = MakeFrustumPlane(row3[0] + row0[0], row3[1] + row0[1], row3[2] + row0[2], row3[3] + row0[3]);
    frustum.planes[1] = MakeFrustumPlane(row3[0] - row0[0], row3[1] - row0[1], row3[2] - row0[2], row3[3] - row0[3]);
    frustum.planes[2] = MakeFrustumPlane(row3[0] - row1[0], row3[1] - row1[1], row3[2] - row1[2], row3[3] - row1[3]);
    frustum.planes[3] = MakeFrustumPlane(row3[0] + row1[0], row3[1] + row1[1], row3[2] + row1[2], row3[3] + row1[3]);
    frustum.planes[4] = MakeFrustumPlane(row3[0] + row2[0], row3[1] + row2[1], row3[2] + row2[2], row3[3] + row2[3]);
    frustum.planes[5] = MakeFrustumPlane(row3[0] - row2[0], row3[1] - row2[1], row3[2] - row2[2], row3[3] - row2[3]);

    return frustum;
}

void ShrinkFrustum(Frustum& frustum, float inset)
{
    for (uint32_t index = 0; index < 4; ++index)
    {
        frustum.planes[index].distance -= inset;
    }
}

bool IntersectsFrustumAABB(const Frustum& frustum, const AABB& aabb)
{
    for (uint32_t index = 0; index < 6; ++index)
    {
        const Plane& plane = frustum.planes[index];

        // Positive vertex (p-vertex): the AABB corner furthest along the plane normal
        Vector3 p_vertex;
        p_vertex.x = plane.normal.x >= 0.0f ? aabb.max.x : aabb.min.x;
        p_vertex.y = plane.normal.y >= 0.0f ? aabb.max.y : aabb.min.y;
        p_vertex.z = plane.normal.z >= 0.0f ? aabb.max.z : aabb.min.z;

        if (Math::DotProduct(plane.normal, p_vertex) + plane.distance < 0.0f)
        {
            // AABB is entirely on the outside half-space of this plane
            return false;
        }
    }

    return true;
}

AABB TransformAABB(const Matrix4x4& matrix, const AABB& aabb)
{
    // Arvo's method: transform the center and extents using absolute matrix values
    // to produce a tight axis-aligned bounding box after transformation.
    const float* m = &matrix.Data[0][0];

    // Center of the original AABB
    const float cx = (aabb.min.x + aabb.max.x) * 0.5f;
    const float cy = (aabb.min.y + aabb.max.y) * 0.5f;
    const float cz = (aabb.min.z + aabb.max.z) * 0.5f;

    // Half-extents of the original AABB
    const float ex = (aabb.max.x - aabb.min.x) * 0.5f;
    const float ey = (aabb.max.y - aabb.min.y) * 0.5f;
    const float ez = (aabb.max.z - aabb.min.z) * 0.5f;

    // Transform center (column-major: Data[Column][Row])
    const float new_cx = m[0] * cx + m[4] * cy + m[8]  * cz + m[12];
    const float new_cy = m[1] * cx + m[5] * cy + m[9]  * cz + m[13];
    const float new_cz = m[2] * cx + m[6] * cy + m[10] * cz + m[14];

    // Transform extents using absolute values of the rotation/scale part
    const float new_ex = Math::Abs(m[0]) * ex + Math::Abs(m[4]) * ey + Math::Abs(m[8])  * ez;
    const float new_ey = Math::Abs(m[1]) * ex + Math::Abs(m[5]) * ey + Math::Abs(m[9])  * ez;
    const float new_ez = Math::Abs(m[2]) * ex + Math::Abs(m[6]) * ey + Math::Abs(m[10]) * ez;

    AABB result;
    result.min = Vector3(new_cx - new_ex, new_cy - new_ey, new_cz - new_ez);
    result.max = Vector3(new_cx + new_ex, new_cy + new_ey, new_cz + new_ez);

    return result;
}


void IntersectRayAABB(const Ray &ray, const AABB &aabb, RayHit& result)
{
    constexpr float epsilon = 1e-6f;

    float tmin = -FLT_MAX;
    float tmax = FLT_MAX;

    Vector3 normal;

    for (uint32_t axis = 0; axis < 3; ++axis)
    {
        const float origin = ray.origin[axis];
        const float direction = ray.direction[axis];
        const float bounds_min = aabb.min[axis];
        const float bounds_max = aabb.max[axis];

        if (Math::Abs(direction) < epsilon)
        {
            if (origin < bounds_min || origin > bounds_max)
            {
                return;
            }

            continue;
        }

        const float inverse = 1.0f / direction;
        
        float t0 = (bounds_min - origin) * inverse;
        float t1 = (bounds_max - origin) * inverse;
        if (t0 > t1)
        {
            std::swap(t0, t1);
        }

        tmin = Math::Max(tmin, t0);
        tmax = Math::Min(tmax, t1);

        if (tmin > tmax)
        {
            return;
        }
    }

    result.hit = true;
    result.distance = tmin;
    result.position = ray.origin + ray.direction * tmin;
}

void IntersectRayTriangle(const Ray &ray, const Vector3 &v0, const Vector3 &v1, const Vector3 &v2, RayHit &result)
{
    constexpr float epsilon = 1e-6f;

    Vector3 edge_1 = v1 - v0;
    Vector3 edge_2 = v2 - v0;

    const Vector3 normal = Math::CrossProduct(edge_1, edge_2);
    if (Math::DotProduct(normal, ray.direction) > 0)
    {
        return;
    }

    const Vector3 h = Math::CrossProduct(ray.direction, edge_2);
    float a = Math::DotProduct(edge_1, h);
    if (Math::Abs(a) < epsilon)
    {
        return;
    }

    float f = 1.0f / a;
    Vector3 s = ray.origin - v0;
    float u = f * Math::DotProduct(s, h);
    if (u < -epsilon || u - 1.0f > epsilon)
    {
        return;
    }

    Vector3 q = Math::CrossProduct(s, edge_1);
    float v = f * Math::DotProduct(ray.direction, q);
    if (v < -epsilon || u + v - 1.0f > epsilon) 
    {
        return;
    }

    result.hit = true;
    result.distance = f * Math::DotProduct(edge_2, q);
    result.position = ray.origin + ray.direction * result.distance;
    result.normal = normal.Normalize();
}

void IntersectRaySurface(const Ray &ray, const Mesh &surface, RayHit &result)
{
    for (uint32_t index = 0; index < surface.faces.size(); ++index)
    {
        const Face& face = surface.faces[index];

        const Vector3& v0 = surface.vertices[face.index_data[0]].position;
        const Vector3& v1 = surface.vertices[face.index_data[1]].position;
        const Vector3& v2 = surface.vertices[face.index_data[2]].position;
        
        RayHit temp_result;
        IntersectRayTriangle(ray, v0, v1, v2, temp_result);

        if (temp_result.hit && temp_result.distance < result.distance)
        {
            result = temp_result;
            result.face = &face;
        }
    }
}

Ray ConstructRayFromMouse(const Vector2 &mouse_position, const Vector2 &screen_extent, const Matrix4x4 &view_matrix, const Matrix4x4 &projection_matrix)
{
    float ndc_x = (2.0f * mouse_position.x / screen_extent.x) - 1.0f;
    float ndc_y = 1.0f - (2.0f * mouse_position.y / screen_extent.y);
    
    // Column-major matrix multiplication
    Matrix4x4 view_proj_inv = (projection_matrix * view_matrix).Inverse();

    Vector4 near_ndc(ndc_x, ndc_y, 1.0f, 1.0f);
    Vector4 far_ndc(ndc_x, ndc_y, 0.1f, 1.0f);

    Vector4 near_world = view_proj_inv * near_ndc;
    Vector4 far_world = view_proj_inv * far_ndc;

    near_world /= near_world.w;
    far_world /= far_world.w;

    Ray ray;
    ray.origin = Vector3(near_world.x, near_world.y, near_world.z);
    ray.direction = (Vector3(far_world.x, far_world.y, far_world.z) - ray.origin).Normalize();

    return ray;
}

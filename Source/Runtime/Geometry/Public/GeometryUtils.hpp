#pragma once

#include "GeometryTypes.hpp"

#include <Math/Matrix.h>

AABB ComputeSurfaceAABB(const Mesh& surface);
float ComputeSurfaceArea(const Mesh& surface);
float ComputeSurfaceVolume(const Mesh& surface);

bool ContainsAABB(const AABB& self, const AABB& other);
bool IntersectsAABB(const AABB& self, const AABB& other);

// Constructs a pyramidal view frustum (left, right, top, bottom, near, far planes)
// from a combined view/projection matrix. Plane normals point towards the
// interior of the frustum.
Frustum ComputeFrustum(const Matrix4x4& view_matrix, const Matrix4x4& projection_matrix);

// Moves frustum planes along their (unit) normals. inset > 0 shrinks (stricter
// cull); inset < 0 expands. Affects left, right, top, and bottom only.
void ShrinkFrustum(Frustum& frustum, float inset);

// Tests whether an AABB intersects with (or is fully contained by) a frustum.
// Uses the positive-vertex (p-vertex) method against each frustum plane.
bool IntersectsFrustumAABB(const Frustum& frustum, const AABB& aabb);

// Transforms an AABB by a matrix, producing a new axis-aligned bounding box
// that encloses the original box after transformation (e.g. local to world space).
AABB TransformAABB(const Matrix4x4& matrix, const AABB& aabb);



void IntersectRayAABB(const Ray& ray, const AABB& aabb, RayHit& result);
void IntersectRayTriangle(const Ray& ray, const Vector3& v0, const Vector3& v1, const Vector3& v2, RayHit& result);
void IntersectRaySurface(const Ray& ray, const Mesh& surface, RayHit& result);

Ray ConstructRayFromDirection(const Vector3& origin, const Vector3& direction);
Ray ConstructRayFromLine(const Vector3& start, const Vector3& end);
Ray ConstructRayFromMouse(const Vector2& mouse_position, const Vector2& screen_extent, const Matrix4x4& view_matrix, const Matrix4x4& projection_matrix);
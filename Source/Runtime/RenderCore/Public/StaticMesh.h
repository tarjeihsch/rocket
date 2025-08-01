#pragma once

class FRHIVertexBuffer;
class FRHIIndexBuffer;

class FStaticMesh
{
    FRHIVertexBuffer* VertexBuffer;
    FRHIIndexBuffer* IndexBuffer;
};

struct FStaticMeshComponent
{
    FStaticMesh* Handle;
};
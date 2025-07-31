#pragma once

class FRHIVertexBuffer;
class FRHIIndexBuffer;

struct FStaticMeshComponent
{
    FRHIVertexBuffer* VertexBuffer;
    FRHIIndexBuffer* IndexBuffer;
};
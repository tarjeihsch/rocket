#pragma once

class FRHIVertexBuffer;
class FRHIIndexBuffer;

struct FPrimitiveDrawInterface
{
};

struct FMeshDrawCommand
{
    FRHIVertexBuffer* VertexBuffer;
    FRHIIndexBuffer* IndexBuffer;
};

class FSpriteDrawCommand : FPrimitiveDrawInterface
{
};
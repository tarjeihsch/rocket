#pragma once

class FStaticMesh;
class FRHIVertexBuffer;
class FRHIIndexBuffer;

struct FPrimitiveDrawInterface
{
};

struct FMeshDrawCommand
{
    FStaticMesh* StaticMesh;
};

class FSpriteDrawCommand : FPrimitiveDrawInterface
{
};
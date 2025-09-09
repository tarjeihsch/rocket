#pragma enable-extensions SPV_EXT_descriptor_indexing

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct FDrawParams
{
    uint DrawParam_1;
    uint DrawParam_2;
    uint DrawParam_3;
    uint DrawParam_4;
};

[[vk::push_constant]]
FDrawParams DrawParams;

struct FCamera
{
    float4x4 View;
    float4x4 Projection;
};

struct FInstance
{
    float4x4 Transform;
    float4x4 TransformInverseTranspose;
};

StructuredBuffer<FCamera> SB_Alias_Camera[] : register(t3, space0);
StructuredBuffer<FInstance> SB_Alias_Instance[] : register(t3, space0);

VSOutput main(VSInput Input)
{
    VSOutput Out;

    FCamera Camera = SB_Alias_Camera[NonUniformResourceIndex(DrawParams.DrawParam_1)][0];
    FInstance Instance = SB_Alias_Instance[NonUniformResourceIndex(DrawParams.DrawParam_2)][0];

    float4 WorldSpacePosition = mul(Instance.Transform, float4(Input.Position, 1.0f));
    float4 ViewPosition = mul(Camera.View, WorldSpacePosition);
    float4 Position = mul(Camera.Projection, ViewPosition);
    float3 Normal = mul(Instance.TransformInverseTranspose, float4(Input.Normal, 0.0f)).xyz;

    Out.Position = Position;
    Out.Normal = Normal;
    Out.TexCoord = Input.TexCoord;

    return Out;
}
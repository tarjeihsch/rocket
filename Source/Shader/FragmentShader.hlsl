struct PSInput
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

float4 main(PSInput Input) : SV_Target
{
    float3 NormalColor = normalize(Input.Normal) * 0.5f + 0.5f;

    return float4(NormalColor, 1.0);
}

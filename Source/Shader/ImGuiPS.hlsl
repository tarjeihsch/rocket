SamplerState   g_Samplers[] : register(s1, space0);
Texture2D      g_Textures[] : register(t0, space0);

struct PSIn {
    float4 pos   : SV_Position;
    float2 uv    : TEXCOORD0;
    float4 color : COLOR0;
};

struct FDrawParams
{
    uint texId;
    uint sampId;
    uint tras;
    uint th;
    float uScaleX;
    float uScaleY;
    float uTranslateX;
    float uTranslateY;
};

[[vk::push_constant]]
FDrawParams DrawParams;

float4 main(PSIn i) : SV_Target {
    return i.color * g_Textures[DrawParams.texId].Sample(g_Samplers[DrawParams.sampId], i.uv);
}

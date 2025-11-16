struct VSIn {
    float2 pos   : POSITION;
    float2 uv    : TEXCOORD0;
    float4 color : COLOR0;    // UNorm8x4 -> float4
};

struct VSOut {
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

struct FView
{
    float2 Scale;
    float2 Translate;
};

StructuredBuffer<FView> SB_Alias_View[] : register(t3, space0);

VSOut main(VSIn i) {
    VSOut o;

    FView View = SB_Alias_View[NonUniformResourceIndex(DrawParams.tras)][0];

    float2 p = i.pos * float2(View.Scale.x, View.Scale.y) + float2(View.Translate.x, View.Translate.y);
    o.pos = float4(p, 0.0f, 1.0f);
    o.uv = i.uv;
    o.color = i.color;
    return o;
}
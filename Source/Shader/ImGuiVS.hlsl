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
    float uScaleX;
    float uScaleY;
    float uTranslateX;
    float uTranslateY;
    float texId;
    float sampId;
};

[[vk::push_constant]]
FDrawParams DrawParams;

VSOut main(VSIn i) {
    VSOut o;
    float2 p = i.pos * float2(DrawParams.uScaleX, DrawParams.uScaleY) + float2(DrawParams.uTranslateX, DrawParams.uTranslateY);
    o.pos = float4(p, 0.0f, 1.0f);
    o.uv = i.uv;
    o.color = i.color;
    return o;
}
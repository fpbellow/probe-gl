struct VS_INPUT
{
    float3 position : POSITION;

};

struct VS_OUTPUT
{
    float4 position : SV_Position;
    float3 Texture : TEXCOORD0;
};

cbuffer PerFrame : register(b0)
{
    matrix view;
    matrix projection;
    float4 viewPos;

};

VS_OUTPUT Main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    matrix viewprojection = mul(projection, view);
    output.position = mul(viewprojection, float4(input.position + viewPos.xyz, 1.0));
    output.Texture = input.position;
    return output;
}

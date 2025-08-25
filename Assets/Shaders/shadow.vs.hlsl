struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 Texture : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 position : SV_Position;
};


cbuffer Light : register(b0)
{
    matrix lightView;
    matrix lightProj;
    float4 lightPos;
};

cbuffer PerObject : register(b1)
{
    matrix modelmatrix;
    matrix invTranspose;
};

VS_OUTPUT Main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    float4 vPos = float4(input.position, 1.0);
    float4 world = mul(modelmatrix, vPos);
    matrix lightViewProj = mul(lightProj, lightView);
    output.position = mul(lightViewProj, world);
    return output;
}

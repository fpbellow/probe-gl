struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 texCoords : TEXCOORD0;
    
};

struct VS_OUTPUT
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 texCoords : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
    float3 lightDir : TEXCOORD2;
    float4 shadowCoord : TEXCOORD3;
    float4 worldPos : WORLDPOS;
};

cbuffer PerFrame : register(b0)
{
    matrix view;
    matrix projection;
    float4 viewPos;

};

cbuffer Light : register(b1)
{
    matrix lightView;
    matrix lightProj;
    float4 lightPos;
};

cbuffer PerObject : register(b2)
{
    matrix modelmatrix;
    matrix invTranspose;
};

VS_OUTPUT Main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    matrix viewprojection = mul(projection, view);
    matrix lightViewProj = mul(lightProj, lightView);
    
    float4 vPos = float4(input.position, 1.0);
    float4 world = mul(modelmatrix, vPos);
    
    output.shadowCoord = mul(lightViewProj, world);
    
    output.normal = normalize(mul(invTranspose, float4(input.normal, 0.0)).xyz);
    output.texCoords = input.texCoords;
    output.tangent = float4(normalize(mul(invTranspose, float4(input.tangent.xyz, 0.0)).xyz), input.tangent.w);
 
    output.viewDir = normalize(viewPos.xyz - world.xyz);
    
    output.lightDir = normalize(lightPos);
    
    output.worldPos = world;
    output.position = mul(viewprojection, world);
    return output;
}
struct GS_INPUT
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

struct GS_OUTPUT
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 texCoords : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
    float3 lightDir : TEXCOORD2;
    float4 shadowCoord : TEXCOORD3;
    uint cubeMapIdx : CUBEMAPIDX;
};


cbuffer ProbeVolumeData : register(b0)
{
    float3 volumeOrigin;
    float3 volumeScale;
    int3 volumeResolution;
};

[maxvertexcount(3)]
void Main(triangle GS_INPUT input[3], inout TriangleStream<GS_OUTPUT> triStream)
{
    for (int i = 0; i < 3; ++i)
    {
        GS_OUTPUT output;
        output.position = input[i].position;
        
        float3 rel = (input[i].worldPos.xyz - volumeOrigin) / volumeScale;
        int x = clamp(rel.x * volumeResolution.x, 0, volumeResolution.x - 1);
        int y = clamp(rel.y * volumeResolution.y, 0, volumeResolution.y - 1);
        int z = clamp(rel.z * volumeResolution.z, 0, volumeResolution.z - 1);
        
        output.cubeMapIdx = x + y * volumeResolution.x + z * volumeResolution.x * volumeResolution.y;
        
        output.normal = input[i].normal;
        output.tangent = input[i].tangent;
        output.texCoords = input[i].texCoords;
        output.viewDir = input[i].viewDir;
        output.lightDir = input[i].lightDir;
        output.shadowCoord = input[i].shadowCoord;
        
        triStream.Append(output);
    }
    triStream.RestartStrip();
}
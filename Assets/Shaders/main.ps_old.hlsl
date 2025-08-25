struct PSInput
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 Texture : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
    float3 lightDir : TEXCOORD2;
    float4 shadowCoord : TEXCOORD3;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

Texture2D baseColorTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D metallicRoughnessTexture : register(t2);

sampler samplerState : register(s0);


Texture2D shadowMap : register(t4);
SamplerComparisonState shadowSampler : register(s1);

PSOutput Main(PSInput input)
{
    PSOutput output = (PSOutput) 0;
   
    /*float2 shadowTexCoords;
    shadowTexCoords.x = 0.5f + (input.shadowCoord.x / input.shadowCoord.w * 0.5f);
    shadowTexCoords.y = 0.5f - (input.shadowCoord.y / input.shadowCoord.w * 0.5f);
    float pixelDepth = input.shadowCoord.z / input.shadowCoord.w;

    float shadow = 1;*/
    float NdotL = dot(input.normal, input.lightDir);
    
    /*if ((saturate(shadowTexCoords.x) == shadowTexCoords.x) &&
        (saturate(shadowTexCoords.y) == shadowTexCoords.y) &&
        (pixelDepth > 0))
    {
        float margin = acos(saturate(NdotL));
        
        float bias = max(0.002 * (1.0f - NdotL), 0.0001);
        bias = clamp(bias, 0, 0.1);
        shadow = (shadowMap.SampleCmpLevelZero(shadowSampler, shadowTexCoords, pixelDepth + bias)).r;
    }*/
    
    //ambient
    float ambient = 0.03f;
    
    //Diffuse component
   
    
    float4 baseColorTex = baseColorTexture.Sample(samplerState, input.Texture);
  
    
    //Specular 
    float3 halfVec = normalize(input.lightDir + input.viewDir);
    
    

    //float3 shading =  ambient;

    output.color = baseColorTex;
    return output;
}
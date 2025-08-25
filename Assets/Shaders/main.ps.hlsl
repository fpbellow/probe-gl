//reference: https://seblagarde.wordpress.com/wp-content/uploads/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf

struct PS_INPUT
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

struct PS_OUTPUT
{
    float4 color : SV_Target0;
};

#define PI 3.14159265359f

Texture2D baseColorTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D metallicRoughnessTexture : register(t2);
sampler samplerLinear : register(s0);


Texture2D shadowMap : register(t3);
SamplerComparisonState shadowSampler : register(s1);

TextureCubeArray<float4> irrCubeProbes : register(t4);

cbuffer PBRData : register(b0)
{
    float4 baseColorFactor;
    float metallicFactor;
    float alphaCutoff;
    int alphaMode;

};

float3 GetNormal(float3 normal, float4 tangent, float2 texCoord)
{
    float3 bitT = cross(normal, tangent.xyz) * tangent.w;
    
    float3 normalMap = normalTexture.Sample(samplerLinear, texCoord).xyz * 2.0 - 1.0;
    float3 tNormal = normalMap.x * tangent.xyz + normalMap.y * bitT + normalMap.z * normal;
    return normalize(tNormal);
}

//Fresnel Schlick approximation
float3 F_Schlick(float3 F0, float u)
{
    return F0 + (1.0 - F0) * pow(1.0 - u, 5);
}

//Normal distribution function (Trowbridge-Reitz GGX)
float D_GGX(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

// Geometry Schlick-GGX
float G_Schlick(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}


float V_SmithGGXCorrelated(float NdotL, float NdotV, float alphaG)
{
    float alphaG2 = alphaG * alphaG;
    float LambdaV = NdotL * sqrt((-NdotV * alphaG2 + NdotV) * NdotV + alphaG2); 
    float LambdaL = NdotV * sqrt((-NdotL * alphaG2 + NdotL) * NdotL + alphaG2);
    return 0.5 / (LambdaV + LambdaL);
}

float Fr_DisneyDiffuse(float NdotV, float NdotL, float LdotH, float roughness)
{
    float energyBias = lerp(0.0, 0.5, roughness);
    float energyFactor = lerp(1.0, 1.0 / 1.51, roughness);

    float fd90 = energyBias + 2.0 * pow(LdotH, 2.0) * roughness;
    float lightScatter = 1.0 + (fd90 - 1.0) * pow(1.0 - NdotL, 5.0);
    float viewScatter = 1.0 + (fd90 - 1.0) * pow(1.0 - NdotV, 5.0);
    return lightScatter * viewScatter * energyFactor;
}

//shadow functions
float2 VogelDiskSample(int sampleIndex, int samplesCount, float phi)
{
    float gAngle = 2.4;
    float r = sqrt(sampleIndex + 0.5) / sqrt(samplesCount);
    float theta = sampleIndex * gAngle + phi;
    
    float sine, cosine;
    sincos(theta, sine, cosine);
    return float2(r * cosine, r * sine);
}

float ComputeShadow(float2 shadowTexCoords,float NdotL, float roughness, float pixelDepth)
{
    float bias = max(0.002 * (1.0f - NdotL), 0.0001);
    bias = clamp(bias, 0, 0.1);
    
    float penumbra = lerp(0.5, 2.5, roughness);
    
    int PCF_SAMPLES = 16;
    float phi = frac(shadowTexCoords.x * 0.7543 + shadowTexCoords.y * 0.5627); //random rotation
    float shadow = 0.0;

    [unroll]
    for (int i = 0; i < PCF_SAMPLES; ++i)
    {
        float2 offset = VogelDiskSample(i, PCF_SAMPLES, phi) * penumbra / 4096;
        float2 uv = shadowTexCoords + offset;
        shadow += shadowMap.SampleCmpLevelZero(shadowSampler, uv, pixelDepth + bias).r;
    }
    return shadow / PCF_SAMPLES;
}

PS_OUTPUT Main(PS_INPUT input)
{
    PS_OUTPUT output = (PS_OUTPUT) 0;
    
    float3 N = GetNormal(input.normal, input.tangent, input.texCoords);
    float3 H = normalize(input.viewDir + input.lightDir);
    float NdotV = abs(dot(N, input.viewDir)) + 1e-5f;
    float NdotL = saturate(dot(N, input.lightDir));
    float NdotH = saturate(dot(N, H));
    float LdotH = saturate(dot(input.lightDir, H));
   
    //sample textures
    float4 baseColorSample = baseColorTexture.Sample(samplerLinear, input.texCoords);
    float4 metalRoughSample = metallicRoughnessTexture.Sample(samplerLinear, input.texCoords);

    //apply factors
    float3 albedo = baseColorSample.rgb * baseColorFactor.xyz;
    float metallic = saturate(metalRoughSample.b * metallicFactor);
    float roughness = max(saturate(metalRoughSample.g * 1.0), 0.425);
    
    //shadows
    float2 shadowTexCoords;
    shadowTexCoords.x = 0.5f + (input.shadowCoord.x / input.shadowCoord.w * 0.5f);
    shadowTexCoords.y = 0.5f - (input.shadowCoord.y / input.shadowCoord.w * 0.5f);
    float pixelDepth = input.shadowCoord.z / input.shadowCoord.w;
    
    float shadow = 1;
    
    if ((saturate(shadowTexCoords.x) == shadowTexCoords.x) &&
        (saturate(shadowTexCoords.y) == shadowTexCoords.y) &&
        (pixelDepth > 0))
    {
        
        shadow = ComputeShadow(shadowTexCoords, NdotL, roughness, pixelDepth);
    }
    
    //compute f0
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
    
    //Specular
    float3 kS = F_Schlick(F0, LdotH);
    float3 Vis = V_SmithGGXCorrelated(NdotL, NdotV, roughness);
    float D = D_GGX(NdotH, roughness);
    float3 Fr = D * kS * Vis / PI;

    //Diffuse
    float3 kD = (1.0 - kS) * (1.0 - metallic);
    float Fd = Fr_DisneyDiffuse(NdotV, NdotL, LdotH, roughness);
    
    float3 lightColor = float3(1.0, 0.85, 0.75); // warm light (soft orange)
    float lightIntensity = 15.0;
    float3 radiance = lightColor * lightIntensity; 
    
    float3 irradiance = irrCubeProbes.Sample(samplerLinear, float4(N, input.cubeMapIdx)).rgb;

    float exposure = 90.0;
    float3 Lo = (kD * albedo / PI * Fd + Fr) * radiance * NdotL;
  
    float3 ambient = max(irradiance * albedo * exposure, albedo * 0.001);

    
  
    float3 color = Lo * shadow + ambient;
  
    //alpha handling
    float alpha = baseColorSample.a;
    if(alphaMode == 1 && alpha < alphaCutoff)
        discard;
    
    float3 mappedColor = color / (color + 1.0);

    output.color = float4(pow(mappedColor, 1.0 / 2.2), alpha);
    return output;
}
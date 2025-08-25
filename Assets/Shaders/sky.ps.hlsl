struct PS_INPUT
{
    float4 position : SV_Position;
    float3 Texture : TEXCOORD0;
};


struct PS_OUTPUT
{
    float4 color : SV_Target0;
};

TextureCube skymap : register(t0);
sampler samplerState : register(s0);

PS_OUTPUT Main(PS_INPUT input)
{
    PS_OUTPUT output = (PS_OUTPUT) 0;
    output.color = skymap.Sample(samplerState, input.Texture);
    return output;
}
#include "RootSignature.hlsl"

float3 color : register(b0);

[RootSignature(ROOTSIG)]
float4 main() : SV_Target
{
    //float3 color = float3(1.f, 0.f, 0.f);
    float attenuation = 1.f;
    return float4(color * attenuation, 1.0f);
}
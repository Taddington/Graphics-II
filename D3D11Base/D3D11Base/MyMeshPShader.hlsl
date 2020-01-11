Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer SHADER_VARS : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
	float4 lightDir;
	float4 lightColor;
};

struct OutputVertex
{
	float4 xyzw : SV_POSITION; // system value
	float4 uvws : OTEXTURE;
	float4 nrms : ONORMAL;
	float4 rgba : OCOLOR;
};

float4 main(OutputVertex inputPixel) : SV_TARGET
{
    float4 finalColor = 0;
	finalColor += saturate(dot(lightDir, inputPixel.nrms) * lightColor);
	finalColor += float4(0.3f, 0.3f, 0.3f, 0.0f);
    finalColor *= txDiffuse.Sample(samLinear, inputPixel.uvws);
    finalColor.a = 1;
    return finalColor;
}
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer SHADER_VARS : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
	float4 directionalLightPos;
	float4 directionalLightColor;
	float4 pointLightPos;
	float4 pointLightColor;
	float4 pointLightRadius;
	float4 time;
	float4 spotLightPos;
	float4 spotLightColor;
	float4 spotLightDir;
	float4 spotLightOuterConeRatio;
	float4 spotLightInnerConeRatio;
};

struct OutputVertex
{
	float4 xyzw : SV_POSITION; // system value
	float4 uvws : OTEXTURE;
	float4 nrms : ONORMAL;
	float4 rgba : OCOLOR;
	float4 worldpos : WORLD;
};

float4 main(OutputVertex inputPixel) : SV_TARGET
{
	return txDiffuse.Sample(samLinear, inputPixel.uvws);
}
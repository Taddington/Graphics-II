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
	float pointAttenuation = 1.0f - saturate(length(pointLightPos - inputPixel.xyzw) / pointLightRadius.x);
	float4 pointLightDirection = normalize(pointLightPos - inputPixel.xyzw);
	float pointLightRatio = saturate(dot(pointLightDirection, inputPixel.nrms) * pointAttenuation);
	float4 pointResult = pointLightRatio * pointLightColor;
	float4 finalColor = 0;
	finalColor += saturate(dot(directionalLightPos, inputPixel.nrms) * directionalLightColor);
	finalColor = saturate(finalColor + float4(0.3f, 0.3f, 0.3f, 0.0f));
	finalColor += pointResult;
	finalColor *= txDiffuse.Sample(samLinear, inputPixel.uvws);
	finalColor.a = 1;
	return finalColor;
}
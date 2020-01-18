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
	float pointAttenuation = 1.0f - saturate(length(pointLightPos - inputPixel.worldpos) / pointLightRadius.x);
	float4 pointLightDirection = normalize(pointLightPos - inputPixel.worldpos);
	float pointLightRatio = saturate(dot(pointLightDirection, inputPixel.nrms) * pointAttenuation);
	float4 pointResult = pointLightRatio * pointLightColor;


	float4 spotDir = normalize(spotLightPos - inputPixel.worldpos);
	float spotSurfaceRatio = saturate(dot(-spotDir, spotLightDir));
	float spotAttenuation = 1.0f - saturate((spotLightInnerConeRatio.x - spotSurfaceRatio) / (spotLightInnerConeRatio.x - spotLightOuterConeRatio.x));
	float spotFactor = (spotSurfaceRatio > spotLightOuterConeRatio.x) ? 1 : 0;
	float spotLightRatio = saturate(dot(spotDir, inputPixel.nrms) * spotAttenuation);
	float4 spotResult = spotLightRatio * spotLightColor;


	float4 finalColor = 0;
	finalColor += saturate(dot(-directionalLightPos, inputPixel.nrms) * directionalLightColor);
	finalColor = saturate(finalColor + float4(0.1f, 0.1f, 0.1f, 0.0f));


	finalColor += pointResult;
	finalColor += spotResult;
	finalColor *= txDiffuse.Sample(samLinear, inputPixel.uvws);
	finalColor.a = 1;
	return finalColor;
}
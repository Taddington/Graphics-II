struct OutputVertex
{
	float4 xyzw : SV_POSITION; // system value
	float4 rgba : OCOLOR;
};

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
};

float4 main(OutputVertex inputPixel) : SV_TARGET
{
	return inputPixel.rgba;
}
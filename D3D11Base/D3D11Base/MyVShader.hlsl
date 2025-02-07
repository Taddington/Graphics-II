// rule of three
// Three things must match
// 1. C++ Vertex Struct
// 2. Input Layout
// 3. HLSL Vertex Struct

//#pragma pack_matrix(row_major)

struct InputVertex
{
	float4 xyzw : POSITION;
	float4 rgba : COLOR;
};

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
	float4 spotLightPos;
	float4 spotLightColor;
	float4 spotLightDir;
	float4 spotLightOuterConeRatio;
	float4 spotLightInnerConeRatio;
};

OutputVertex main(InputVertex input)
{
	OutputVertex output = (OutputVertex)0;
	output.xyzw = input.xyzw;
	output.xyzw.y += sin(input.xyzw.x * 1 * time.x) * 0.1f;
	output.rgba = input.rgba;
	// Do math here (shader intrinsics)

	output.xyzw = mul(worldMatrix, output.xyzw);
	output.xyzw = mul(viewMatrix, output.xyzw);
	output.xyzw = mul(projectionMatrix, output.xyzw);
	// don't do perspective divide

	return output;
}
// rule of three
// Three things must match
// 1. C++ Vertex Struct
// 2. Input Layout
// 3. HLSL Vertex Struct

struct InputVertex
{
	float3 xyz : POSITION;
	float3 nrm : NORMAL;
	float3 uvw : TEXCOORD;
};

struct OutputVertex
{
	float4 xyzw : SV_POSITION; // system value
	float4 uvws : OTEXTURE;
	float4 nrms : ONORMAL;
	float4 rgba : OCOLOR;
	float4 worldpos : WORLD;
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

OutputVertex main(InputVertex input, uint instanceID : SV_InstanceID)
{
	OutputVertex output = (OutputVertex)0;
	output.xyzw = float4(input.xyz, 1);
	output.xyzw.x += (7 * instanceID);
	output.nrms.xyz = input.nrm;
	output.uvws.xyz = input.uvw;
	// Do math here (shader intrinsics)

	output.xyzw = mul(worldMatrix, output.xyzw);
	output.worldpos = output.xyzw;
	output.xyzw = mul(viewMatrix, output.xyzw);
	output.xyzw = mul(projectionMatrix, output.xyzw);
	// don't do perspective divide

	return output;
}
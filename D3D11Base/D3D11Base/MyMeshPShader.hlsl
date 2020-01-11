Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

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
    finalColor = txDiffuse.Sample(samLinear, inputPixel.uvws);
    finalColor.a = 1;
    return finalColor;
}
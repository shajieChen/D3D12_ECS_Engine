cbuffer ConstantBuffer : register(b0)
{
	float4x4 worldViewProjection;
};

struct VertexInput
{
	float3 pos	: POSITION;
	float2 uv	: TEXCOORD;
};

struct VertexOutput
{
	float4 pos	: SV_POSITION;
	float2 uv	: TEXCOORD;
};

VertexOutput main(VertexInput input)
{
	VertexOutput output;
	//output.pos = mul(input.pos, worldViewProjection);
	output.pos = mul(float4(input.pos, 1.0f), worldViewProjection);
	output.uv = input.uv;
	return output;
}
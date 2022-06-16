
cbuffer cbCameraInfo : register(b0)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
	float3		gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b1)
{
	matrix		gmtxGameObject : packoffset(c0);
};

cbuffer cbConstantsInfo : register(b4)
{
	int			gnMaterial : packoffset(c0.x);
	float4		gcAlbedoColor : packoffset(c1);
	float4		gcEmissionColor : packoffset(c2);
};

#include "Light.hlsl"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define _WITH_VERTEX_LIGHTING

struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
#ifdef _WITH_VERTEX_LIGHTING
	float4 color : COLOR;
#endif
};

VS_OUTPUT VSLighting(VS_INPUT input)
{
	VS_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
#ifdef _WITH_VERTEX_LIGHTING
	output.normalW = normalize(output.normalW);
	output.color = Lighting(output.positionW, output.normalW);
#endif
	return(output);
}

float4 PSLighting(VS_OUTPUT input) : SV_TARGET
{
#ifdef _WITH_VERTEX_LIGHTING
	return(input.color);
#else
	input.normalW = normalize(input.normalW);
	float4 color = Lighting(input.positionW, input.normalW);

	return(color);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
static float3 gDirectionalLight = float3(0.0f, -1.0f, 1.0f);
static float4 gLightColor = float4(0.0135f, 0.035f, 0.357f, 1.0f);

float4 PSPlayer(VS_OUTPUT input) : SV_TARGET
{
	float4 color;
#ifdef _WITH_VERTEX_LIGHTING
	color = input.color;
	return(color);
#else
	float3 normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, normalW);
	cIllumination = saturate(float4(0.12f, 0.12f, 0.12f, 1.0f) + gLightColor * abs(dot(normalize(input.normalW), normalize(-gDirectionalLight))));
	return(cIllumination);
#endif
//	return(cIllumination + gLightColor + gcEmissionColor * 0.15f);
//	return(color * cIllumination * gcAlbedoColor + gcEmissionColor);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//정점 셰이더의 입력을 위한 구조체를 선언한다. 
struct VS_DIFFUSED_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
};
//정점 셰이더의 출력(픽셀 셰이더의 입력)을 위한 구조체를 선언한다. 
struct VS_DIFFUSED_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

//정점 셰이더를 정의한다. 
VS_DIFFUSED_OUTPUT VSDiffused(VS_DIFFUSED_INPUT input)
{
	VS_DIFFUSED_OUTPUT output;
	//정점을 변환(월드 변환, 카메라 변환, 투영 변환)한다. 
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.color = input.color;
	return(output);
}
//픽셀 셰이더를 정의한다. 
float4 PSDiffused(VS_DIFFUSED_OUTPUT input) : SV_TARGET
{
	return(input.color);
}
//���� ��ü�� ������ ���� ��� ���۸� �����Ѵ�. 
cbuffer cbGameObjectInfo : register(b0)
{
	matrix gmtxWorld : packoffset(c0);
};
//ī�޶��� ������ ���� ��� ���۸� �����Ѵ�. 
cbuffer cbCameraInfo : register(b1)
{
	matrix gmtxView : packoffset(c0);
	matrix gmtxProjection : packoffset(c4);
};
//���� ���̴��� �Է��� ���� ����ü�� �����Ѵ�. 
struct VS_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
};
//���� ���̴��� ���(�ȼ� ���̴��� �Է�)�� ���� ����ü�� �����Ѵ�. 
struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

struct VS_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct VS_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float4 color : COLOR;
};

//�ν��Ͻ� �����͸� ���� ����ü�̴�. 
struct INSTANCEDGAMEOBJECTINFO
{
	matrix m_mtxGameObject;
	float4 m_cColor;
};

StructuredBuffer<INSTANCEDGAMEOBJECTINFO> gGameObjectInfos : register(t0);
struct VS_INSTANCING_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
};
struct VS_INSTANCING_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};
VS_INSTANCING_OUTPUT VSInstancing(VS_INSTANCING_INPUT input, uint nInstanceID : SV_InstanceID)
{
	VS_INSTANCING_OUTPUT output;
	output.position = mul(mul(mul(float4(input.position, 1.0f),
		gGameObjectInfos[nInstanceID].m_mtxGameObject), gmtxView), gmtxProjection);
	output.color = input.color + gGameObjectInfos[nInstanceID].m_cColor;
	return(output);
}
float4 PSInstancing(VS_INSTANCING_OUTPUT input) : SV_TARGET
{
	return(input.color);
}

//���� ���̴��� �����Ѵ�. 
VS_OUTPUT VSDiffused(VS_INPUT input)
{
	VS_OUTPUT output;
	//������ ��ȯ(���� ��ȯ, ī�޶� ��ȯ, ���� ��ȯ)�Ѵ�. 
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), 
	gmtxProjection);
	output.color = input.color;
	return(output);
}
//�ȼ� ���̴��� �����Ѵ�. 
float4 PSDiffused(VS_OUTPUT input) : SV_TARGET
{
	return(input.color);
}

//���ο� ���̴��� �����Ѵ�.
VS_LIGHTING_OUTPUT VSPseudoLighting(VS_LIGHTING_INPUT input)
{
	VS_LIGHTING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxWorld);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxWorld);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.color = lerp(float4(normalize(output.positionW), 1.0f), float4(normalize(output.normalW), 1.0f), 0.5f);

	return(output);
}

float4 PSPseudoLighting(VS_LIGHTING_OUTPUT input) : SV_TARGET
{
	return(input.color);
}
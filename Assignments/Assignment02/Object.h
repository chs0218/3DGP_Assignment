//------------------------------------------------------- ----------------------
// File: Object.h
//-----------------------------------------------------------------------------

#pragma once

#include "Mesh.h"
#include "Camera.h"

#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20

#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02	//[]
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE		0x04
#define RESOURCE_BUFFER				0x05

class CShader;
class CTerrainShader;

struct CB_GAMEOBJECT_INFO
{
	XMFLOAT4X4						m_xmf4x4World;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct SRVROOTARGUMENTINFO
{
	int								m_nRootParameterIndex = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGpuDescriptorHandle;
};

class CTexture
{
public:
	CTexture(int nTextureResources = 1, UINT nResourceType = RESOURCE_TEXTURE2D, int nSamplers = 0);
	virtual ~CTexture();

private:
	int								m_nReferences = 0;

	UINT							m_nTextureType = RESOURCE_TEXTURE2D;

	int								m_nTextures = 0;
	ID3D12Resource					**m_ppd3dTextures = NULL;
	ID3D12Resource					**m_ppd3dTextureUploadBuffers;

	int								m_nSamplers = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		*m_pd3dSamplerGpuDescriptorHandles = NULL;

public:
	SRVROOTARGUMENTINFO				*m_pRootArgumentInfos = NULL;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void SetRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dsrvGpuDescriptorHandle);
	void SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, int nIndex);
	void ReleaseShaderVariables();

	void LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, wchar_t *pszFileName, UINT nIndex);

	int GetTextures() { return(m_nTextures); }
	ID3D12Resource *GetTexture(int nIndex) { return(m_ppd3dTextures[nIndex]); }
	UINT GetTextureType() { return(m_nTextureType); }

	void ReleaseUploadBuffers();
};

class CMaterial
{
public:
	CMaterial();
	virtual ~CMaterial();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	CTexture						*m_pAlbedoTexture = NULL;

	XMFLOAT4						m_xmf4EmissionColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	CTexture						*m_pEmissionTexture = NULL;

	int								m_nMaterial = 1; //Material Index, CScene::m_pReflections[]

	void SetAlbedoColor(XMFLOAT4 xmf4Color) { m_xmf4AlbedoColor = xmf4Color; }
	void SetEmissionColor(XMFLOAT4 xmf4Color) { m_xmf4EmissionColor = xmf4Color; }
	void SetMaterial(int nMaterial) { m_nMaterial = nMaterial; }
	void SetAlbedoTexture(CTexture *pTexture);
	void SetEmissionTexture(CTexture *pTexture);

	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	void ReleaseShaderVariables();

	void ReleaseUploadBuffers();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CGameObject
{
public:
	CGameObject(UINT nMaterials=1);
	virtual ~CGameObject();

public:
	char							m_pstrName[64] = { '\0' };

	XMFLOAT4X4						m_xmf4x4World;
	CMesh							*m_pMesh = NULL;
	CShader							*m_pShader = NULL;

	UINT							m_nMaterials = 0; 
	CMaterial						**m_ppMaterials = NULL;

protected:
	ID3D12Resource					*m_pd3dcbGameObject = NULL;

public:
	CB_GAMEOBJECT_INFO				*m_pcbMappedGameObject = NULL;

public:
	void SetMesh(CMesh *pMesh);
	void SetShader(CShader *pShader);
	void SetAlbedoColor(UINT nIndex, XMFLOAT4 xmf4Color);
	void SetEmissionColor(UINT nIndex, XMFLOAT4 xmf4Color);
	void SetMaterial(UINT nIndex, CMaterial *pMaterial);
	void SetMaterial(UINT nIndex, UINT nMaterial);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void Animate(float fTimeElapsed) { }
	virtual void OnPrepareRender() { }
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera=NULL);

	virtual void ReleaseUploadBuffers();

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);

	void LoadGameObjectFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName);
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MyGameObject
{
public:
	MyGameObject(int nMeshes = 1);
	virtual ~MyGameObject();
private:
	int m_nReferences = 0;
public:
	void OnPrepareRender();
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void SetMesh(int nIndex, CMesh* pMesh);
	void SetShader(CShader* pShader);
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
public:
	XMFLOAT4X4 m_xmf4x4World;
	CShader* m_pShader = NULL;
	//게임 객체는 여러 개의 메쉬를 포함하는 경우 게임 객체가 가지는 메쉬들에 대한 포인터와 그 개수이다. 
	CMesh** m_ppMeshes = NULL;
	int m_nMeshes = 0;
public:
	
};

class CHeightMapTerrain : public MyGameObject
{
public:
	CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, 
		int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color);
	virtual ~CHeightMapTerrain();
private:
	//지형의 높이 맵으로 사용할 이미지이다. 
	CHeightMapImage* m_pHeightMapImage;
	//높이 맵의 가로와 세로 크기이다. 
	int m_nWidth;
	int m_nLength;
	//지형을 실제로 몇 배 확대할 것인가를 나타내는 스케일 벡터이다. 
	XMFLOAT3 m_xmf3Scale;
public:
	//지형의 높이를 계산하는 함수이다(월드 좌표계). 높이 맵의 높이에 스케일의 y를 곱한 값이다. 
	float GetHeight(float x, float z) { return(m_pHeightMapImage->GetHeight(x / m_xmf3Scale.x, z / m_xmf3Scale.z) * m_xmf3Scale.y); }
	//지형의 법선 벡터를 계산하는 함수이다(월드 좌표계). 높이 맵의 법선 벡터를 사용한다. 
	XMFLOAT3 GetNormal(float x, float z) {
		return(m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x), int(z / m_xmf3Scale.z)));
	}
	int GetHeightMapWidth() { return(m_pHeightMapImage->GetHeightMapWidth()); }
	int GetHeightMapLength() { return(m_pHeightMapImage->GetHeightMapLength()); }
	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	//지형의 크기(가로/세로)를 반환한다. 높이 맵의 크기에 스케일을 곱한 값이다. 
	float GetWidth() { return(m_nWidth * m_xmf3Scale.x); }
	float GetLength() { return(m_nLength * m_xmf3Scale.z); }
};
//------------------------------------------------------- ----------------------
// File: Object.h
//-----------------------------------------------------------------------------

#pragma once

#include "Mesh.h"
#include "Camera.h"
#include "stdafx.h"

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

//========================================================================================================
class CMyShader;

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40
#define EXPLOSION_DEBRISES 10

struct MATERIALLOADINFO
{
	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	float							m_fGlossiness = 0.0f;
	float							m_fSmoothness = 0.0f;
	float							m_fSpecularHighlight = 0.0f;
	float							m_fMetallic = 0.0f;
	float							m_fGlossyReflection = 0.0f;

	UINT							m_nType = 0x00;

	TCHAR							m_pstrAlbedoMapName[128] = { '\0' };
	TCHAR							m_pstrSpecularMapName[128] = { '\0' };
	TCHAR							m_pstrMetallicMapName[128] = { '\0' };
	TCHAR							m_pstrNormalMapName[128] = { '\0' };
	TCHAR							m_pstrEmissionMapName[128] = { '\0' };
	TCHAR							m_pstrDetailAlbedoMapName[128] = { '\0' };
	TCHAR							m_pstrDetailNormalMapName[128] = { '\0' };
};

class CMyObject
{
public:
	CMyObject();
	virtual ~CMyObject();
private:
	int m_nReferences = 0;
public:
	BoundingOrientedBox			m_xmOOBB = BoundingOrientedBox();
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
public:
	//상수 버퍼를 생성한다. 
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	//상수 버퍼의 내용을 갱신한다. 
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	//게임 객체의 월드 변환 행렬에서 위치 벡터와 방향(x-축, y-축, z-축) 벡터를 반환한다. 
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();
	XMFLOAT4X4 Getxmf4x4World() const;
	//게임 객체의 위치를 설정한다. 
	virtual void SetPosition(float x, float y, float z);
	virtual void SetPosition(XMFLOAT3 xmf3Position);
	void SetMovingDirection(XMFLOAT3& xmf3MovingDirection);
	void SetMovingSpeed(float fSpeed) { m_fMovingSpeed = fSpeed; }
	//게임 객체를 로컬 x-축, y-축, z-축 방향으로 이동한다.
	virtual void MoveStrafe(float fDistance = 1.0f);
	virtual void MoveUp(float fDistance = 1.0f);
	virtual void MoveForward(float fDistance = 1.0f);
	void Move()
	{
		SetPosition(m_xmf4x4World._41 + m_xmf3MovingDirection.x * m_fMovingSpeed, m_xmf4x4World._42 + m_xmf3MovingDirection.y * m_fMovingSpeed, m_xmf4x4World._43 + m_xmf3MovingDirection.z * m_fMovingSpeed);
	}
	//게임 객체를 회전(x-축, y-축, z-축)한다. 
	virtual void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CMyCamera* pCamera, UINT nInstances);
protected:
	XMFLOAT4X4 m_xmf4x4World;
	CMyShader* m_pShader = NULL;
public:
	CMyMesh* m_pMesh = NULL;
	XMFLOAT3 m_xmf3MovingDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);
	float m_fMovingSpeed = 0.0f;
	bool Show = true;
	void ReleaseUploadBuffers();
	virtual void SetMesh(CMyMesh* pMesh);
	virtual void SetShader(CMyShader* pShader);
	virtual void Animate(float fTimeElapsed);
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CMyCamera* pCamera);
	void UpdateBoundingBox()
	{
		if (m_pMesh)
		{
			m_pMesh->m_xmOOBB.Transform(m_xmOOBB, XMLoadFloat4x4(&m_xmf4x4World));
			XMStoreFloat4(&m_xmOOBB.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBB.Orientation)));
		}
	}

	virtual bool checkCollision(BoundingOrientedBox m_OB)
	{
		if (m_pMesh)
		{
			if (m_xmOOBB.Intersects(m_OB))
				return true;
		}
		return false;
	}
};

class CRotatingObject : public CMyObject
{
public:
	CRotatingObject();
	virtual ~CRotatingObject();
private:
	XMFLOAT3 m_xmf3RotationAxis;
	float m_fRotationSpeed;

public:
	void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; }
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) {
		m_xmf3RotationAxis =
			xmf3RotationAxis;
	}
	virtual void Animate(float fTimeElapsed);
};

class CTrackObject : public CMyObject
{
public:
	CTrackObject() {};
	virtual ~CTrackObject() {};
public:
	virtual void Animate(float fTimeElapsed) {};
};

struct MATERIALSLOADINFO
{
	int	m_nMaterials = 0;
	MATERIALLOADINFO* m_pMaterials = NULL;

};

class CMyMaterial
{
public:
	CMyMaterial();
	virtual ~CMyMaterial();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	CMyShader* m_pShader = NULL;

	void SetShader(CMyShader* pShader);
	void SetPseudoLightingShader() { SetShader(m_pPseudoLightingShader); }

	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList);

protected:
	static CMyShader* m_pPseudoLightingShader;

public:
	static void CMyMaterial::PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
};

class CHierarchyObject : public CMyObject
{
public:
	TCHAR m_pstrFrameName[256];
	XMFLOAT4X4 m_xmf4x4Transform;
	CHierarchyObject* m_pParent = NULL;
	CHierarchyObject* m_pChild = NULL;
	CHierarchyObject* m_pSibling = NULL;
	int	m_nMaterials = 0;
	CMyMaterial** m_ppMaterials = NULL;
public:
	CHierarchyObject();
	virtual ~CHierarchyObject();
	void SetChild(CHierarchyObject* pChild);
	void SetMaterial(int nMaterial, CMyMaterial* pMaterial);
	CHierarchyObject* FindFrame(_TCHAR* pstrFrameName);

	virtual void SetPosition(float x, float y, float z)
	{
		m_xmf4x4Transform._41 = x;
		m_xmf4x4Transform._42 = y;
		m_xmf4x4Transform._43 = z;
		UpdateTransform(NULL);
	}

	virtual void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void SetScale(float x, float y, float z);
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent);
	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0); }
public:
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
	{
		if (m_pSibling) m_pSibling->Animate(fTimeElapsed, pxmf4x4Parent);
		if (m_pChild) m_pChild->Animate(fTimeElapsed, &m_xmf4x4World);
	}

	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CMyCamera* pCamera = NULL);
	static void PrintFrameInfo(CHierarchyObject* pGameObject, CHierarchyObject* pParent);

	static MATERIALSLOADINFO* LoadMaterialsInfoFromFile(wifstream& InFile);
	static CMeshLoadInfo* LoadMeshInfoFromFile(wifstream& InFile);
	static CHierarchyObject* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, wifstream& InFile);
	static CHierarchyObject* LoadGeometryFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, TCHAR* pstrFileName);
	virtual bool checkCollision(BoundingOrientedBox m_OB)
	{
		if (m_pMesh)
		{
			if (m_xmOOBB.Intersects(m_OB))
				return true;
		}

		if (m_pChild && m_pChild->checkCollision(m_OB))
			return true;
		if (m_pSibling && m_pSibling->checkCollision(m_OB))
			return true;

		return false;
	}
};

//인스턴스 정보(게임 객체의 월드 변환 행렬과 객체의 색상)를 위한 구조체이다. 
struct VS_VB_INSTANCE
{
	XMFLOAT4X4 m_xmf4x4Transform;
	XMFLOAT4 m_xmcColor;
};

class ObstacleObject
{
public:
	ObstacleObject() {};
	~ObstacleObject() {};
	CHierarchyObject* p_Obs;
	vector<CMyObject*> p_Explosions;
	ID3D12Resource* m_myGameObjects = NULL;
	VS_VB_INSTANCE* m_myMappedGameObjects = NULL;
	float duration = 0.0f;
	bool IsExPlosing = false;
	void Release() {};
	void UpdatePosition() {
		p_Obs->SetPosition(p_Obs->GetPosition().x, p_Obs->GetPosition().y, p_Obs->GetPosition().z + 15000.0f);
		for (int i = 0; i < p_Explosions.size(); ++i)
		{
			p_Explosions[i]->SetPosition(p_Obs->GetPosition());
		}
	}
	void Update(float fTimeElapsed) {
		if (IsExPlosing)
		{
			duration += fTimeElapsed;
			if (duration > 2.0f)
			{
				IsExPlosing = false;
				duration = 0.0f;
				for (int i = 0; i < p_Explosions.size(); ++i)
				{
					p_Explosions[i]->SetPosition(p_Obs->GetPosition());
				}
			}

			else
			{
				for (int j = 0; j < p_Explosions.size(); j++)
				{
					p_Explosions[j]->Animate(fTimeElapsed);
					p_Explosions[j]->Move();
				}
			}
		}
	}
	void SetExplosion(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void Render1(ID3D12GraphicsCommandList* pd3dCommandList, CMyCamera* pCamera)
	{
		if (IsExPlosing)
		{
			pd3dCommandList->SetGraphicsRootShaderResourceView(2, m_myGameObjects->GetGPUVirtualAddress());
			for (int j = 0; j < p_Explosions.size(); j++)
			{
				m_myMappedGameObjects[j].m_xmcColor = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
				XMStoreFloat4x4(&m_myMappedGameObjects[j].m_xmf4x4Transform,
					XMMatrixTranspose(XMLoadFloat4x4(&p_Explosions[j]->Getxmf4x4World())));
			}
			p_Explosions[0]->Render(pd3dCommandList, pCamera, p_Explosions.size());
		}
	}

	void Render2(ID3D12GraphicsCommandList* pd3dCommandList, CMyCamera* pCamera)
	{
		if (!IsExPlosing)
			p_Obs->Render(pd3dCommandList, pCamera);
	}
};
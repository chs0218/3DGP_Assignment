#pragma once
#include "Mesh.h"
#include "Camera.h"

class CShader;

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

class CGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();
private:
	int m_nReferences = 0;
public:
	BoundingOrientedBox			m_xmOOBB = BoundingOrientedBox();
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
public:
	//상수 버퍼를 생성한다. 
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	//상수 버퍼의 내용을 갱신한다. 
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
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
	void SetMovingDirection(XMFLOAT3& xmf3MovingDirection) { m_xmf3MovingDirection = Vector3::Normalize(xmf3MovingDirection); }
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
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, UINT nInstances);
protected:
	XMFLOAT4X4 m_xmf4x4World;
	CShader* m_pShader = NULL;
public:
	CMesh* m_pMesh = NULL;
	XMFLOAT3 m_xmf3MovingDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);
	float m_fMovingSpeed = 0.0f;
	bool Show = true;
	void ReleaseUploadBuffers();
	virtual void SetMesh(CMesh* pMesh);
	virtual void SetShader(CShader* pShader);
	virtual void Animate(float fTimeElapsed);
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
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

class CRotatingObject : public CGameObject
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

class CTrackObject : public CGameObject
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

	CShader* m_pShader = NULL;

	void SetShader(CShader* pShader);
	void SetPseudoLightingShader() { SetShader(m_pPseudoLightingShader); }

	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList);

protected:
	static CShader* m_pPseudoLightingShader;

public:
	static void CMaterial::PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
};

class CHierarchyObject : public CGameObject
{
public:
	TCHAR m_pstrFrameName[256];
	XMFLOAT4X4 m_xmf4x4Transform;
	CHierarchyObject* m_pParent = NULL;
	CHierarchyObject* m_pChild = NULL;
	CHierarchyObject* m_pSibling = NULL;
	int	m_nMaterials = 0;
	CMaterial** m_ppMaterials = NULL;
public:
	CHierarchyObject();
	virtual ~CHierarchyObject();
	void SetChild(CHierarchyObject* pChild);
	void SetMaterial(int nMaterial, CMaterial* pMaterial);
	CHierarchyObject* FindFrame(_TCHAR* pstrFrameName);

	virtual void SetPosition(float x, float y, float z)
	{
		m_xmf4x4Transform._41 = x;
		m_xmf4x4Transform._42 = y;
		m_xmf4x4Transform._43 = z;
		UpdateTransform(NULL);
	}

	virtual void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f)
	{
		XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
		m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);
		UpdateTransform(NULL);
	}
	void SetScale(float x, float y, float z)
	{
		XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
		m_xmf4x4Transform = Matrix4x4::Multiply(mtxScale, m_xmf4x4Transform);
		UpdateTransform(NULL);
	}
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
	{
		m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4Transform, *pxmf4x4Parent) : m_xmf4x4Transform;
		UpdateBoundingBox();

		if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
		if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);
	}
	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0); }
public:
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
	{
		if (m_pSibling) m_pSibling->Animate(fTimeElapsed, pxmf4x4Parent);
		if (m_pChild) m_pChild->Animate(fTimeElapsed, &m_xmf4x4World);
	}

	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
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
	vector<CGameObject*> p_Explosions;
	ID3D12Resource* m_myGameObjects = NULL;
	VS_VB_INSTANCE* m_myMappedGameObjects = NULL;
	float duration = 0.0f;
	bool IsExPlosing = false;
	void Release() {};
	void Update(float fTimeElapsed) {
		if (IsExPlosing)
		{
			duration += fTimeElapsed;
			if (duration > 1.0f)
			{
				IsExPlosing = false;
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
	void Render1(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
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

	void Render2(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
	{
		if (!IsExPlosing)
			p_Obs->Render(pd3dCommandList, pCamera);
	}
};
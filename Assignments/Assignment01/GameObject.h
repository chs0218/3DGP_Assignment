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
	//게임 객체를 로컬 x-축, y-축, z-축 방향으로 이동한다.
	virtual void MoveStrafe(float fDistance = 1.0f);
	virtual void MoveUp(float fDistance = 1.0f);
	virtual void MoveForward(float fDistance = 1.0f);
	//게임 객체를 회전(x-축, y-축, z-축)한다. 
	virtual void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, UINT nInstances);
protected:
	XMFLOAT4X4 m_xmf4x4World;
	CMesh* m_pMesh = NULL;
	CShader* m_pShader = NULL;
public:
	void ReleaseUploadBuffers();
	virtual void SetMesh(CMesh* pMesh);
	virtual void SetShader(CShader* pShader);
	virtual void Animate(float fTimeElapsed);
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
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
};

class CTrackObject : public CGameObject
{
public:
	CGameObject* m_ppObjects;
public:
	CTrackObject() {};
	virtual ~CTrackObject() {
		if (m_ppObjects)
			delete m_ppObjects;
	};

	void SetObstacle(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) {
		CHierarchyObject* pObstacleObject = new CHierarchyObject();
		CHierarchyObject* pGameObject = CHierarchyObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Model/Rock.txt");
		pObstacleObject->SetChild(pGameObject);
		pObstacleObject->SetPosition(-50.0f, 0.0f, 150.0f);
		pObstacleObject->SetScale(1.5f, 1.5f, 1.5f);

		m_ppObjects = pObstacleObject;
	}
public:
	virtual void Animate(float fTimeElapsed) {};
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
	{
		CGameObject::Render(pd3dCommandList, pCamera);
		if (m_ppObjects)
			m_ppObjects->Render(pd3dCommandList, pCamera);
	}
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, UINT nInstances)
	{
		CGameObject::Render(pd3dCommandList, pCamera, nInstances);
		if (m_ppObjects)
			m_ppObjects->Render(pd3dCommandList, pCamera);

	}
};
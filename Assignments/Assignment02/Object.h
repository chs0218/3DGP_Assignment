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

class CShader;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

#define ADDITIONAL_SIZE		3
#define PADDING				10.0f

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

	//char							m_pstrAlbedoMapName[64] = { '\0' };
	//char							m_pstrSpecularMapName[64] = { '\0' };
	//char							m_pstrMetallicMapName[64] = { '\0' };
	//char							m_pstrNormalMapName[64] = { '\0' };
	//char							m_pstrEmissionMapName[64] = { '\0' };
	//char							m_pstrDetailAlbedoMapName[64] = { '\0' };
	//char							m_pstrDetailNormalMapName[64] = { '\0' };
};

struct MATERIALSLOADINFO
{
	int								m_nMaterials = 0;
	MATERIALLOADINFO				*m_pMaterials = NULL;
};

class CMaterialColors
{
public:
	CMaterialColors() { }
	CMaterialColors(MATERIALLOADINFO *pMaterialInfo);
	virtual ~CMaterialColors() { }

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	XMFLOAT4						m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	XMFLOAT4						m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); //(r,g,b,a=power)
	XMFLOAT4						m_xmf4Emissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
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

	CShader							*m_pShader = NULL;

	CMaterialColors					*m_pMaterialColors = NULL;

	void SetMaterialColors(CMaterialColors *pMaterialColors);
	void SetShader(CShader *pShader);
	void SetIlluminatedShader() { SetShader(m_pIlluminatedShader); }

	void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList);

protected:
	static CShader					*m_pIlluminatedShader;

public:
	static void CMaterial::PrepareShaders(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
};

class CGameObject
{
private:
	int								m_nReferences = 0;
public:
	void AddRef();
	void Release();
public:
	CGameObject();
    virtual ~CGameObject();
public:
	char							m_pstrFrameName[64];

	CMesh							*m_pMesh = NULL;

	int								m_nMaterials = 0;
	CMaterial						**m_ppMaterials = NULL;

	XMFLOAT4X4						m_xmf4x4Transform;
	XMFLOAT4X4						m_xmf4x4World;

	CGameObject 					*m_pParent = NULL;
	CGameObject 					*m_pChild = NULL;
	CGameObject 					*m_pSibling = NULL;
	bool							isEnable = false;
	BoundingOrientedBox				m_xmOOBB = BoundingOrientedBox();

	void SetMesh(CMesh *pMesh);
	void SetShader(CShader *pShader);
	void SetShader(int nMaterial, CShader *pShader);
	void SetMaterial(int nMaterial, CMaterial *pMaterial);

	void SetChild(CGameObject* pChild, bool bReferenceUpdate = false);

	virtual void BuildMaterials(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) { }

	virtual void OnInitialize() { }
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent=NULL);

	virtual void OnPrepareRender() { }
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera=NULL);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, CMaterial *pMaterial);

	virtual void ReleaseUploadBuffers();
	virtual void Respawn(XMFLOAT3 position);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	void SetScale(float x, float y, float z);

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4 *pxmf4Quaternion);

	CGameObject *GetParent() { return(m_pParent); }
	void UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent=NULL);
	CGameObject *FindFrame(char *pstrFrameName);

	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0); }
	virtual void UpdateBoundingBox()
	{
		if (m_pMesh)
		{
			m_pMesh->m_xmOOBB.Transform(m_xmOOBB, XMLoadFloat4x4(&m_xmf4x4World));
			XMStoreFloat4(&m_xmOOBB.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBB.Orientation)));
		}
	}

	virtual bool checkObjectCollision(CGameObject* target)
	{
		if (target->m_pMesh && checkCollision(target->m_xmOOBB))
			return true;
		if (target->m_pChild && checkObjectCollision(target->m_pChild))
			return true;
		if (target->m_pSibling && checkObjectCollision(target->m_pSibling))
			return true;
		return false;
	}
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
public:
	static MATERIALSLOADINFO *LoadMaterialsInfoFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FILE *pInFile);
	static CMeshLoadInfo *LoadMeshInfoFromFile(FILE *pInFile);

	static CGameObject *LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, FILE *pInFile);
	static CGameObject *LoadGeometryFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, char *pstrFileName);

	static void PrintFrameInfo(CGameObject *pGameObject, CGameObject *pParent);
};

class CRotatingObject : public CGameObject
{
public:
	CRotatingObject();
    virtual ~CRotatingObject();

private:
	XMFLOAT3					m_xmf3RotationAxis;
	float						m_fRotationSpeed;

public:
	void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; }
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) { m_xmf3RotationAxis = xmf3RotationAxis; }

	virtual void Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent=NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
};

class CRevolvingObject : public CGameObject
{
public:
	CRevolvingObject();
	virtual ~CRevolvingObject();

private:
	XMFLOAT3					m_xmf3RevolutionAxis;
	float						m_fRevolutionSpeed;
public:
	void SetRevolutionSpeed(float fRevolutionSpeed) { m_fRevolutionSpeed = fRevolutionSpeed; }
	void SetRevolutionAxis(XMFLOAT3 xmf3RevolutionAxis) { m_xmf3RevolutionAxis = xmf3RevolutionAxis; }

	virtual void Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent=NULL);
};

class CHellicopterObject : public CGameObject
{
public:
	CHellicopterObject(void* pContext = NULL);
	virtual ~CHellicopterObject();
protected:
	CGameObject					*m_pMainRotorFrame = NULL;
	CGameObject					*m_pTailRotorFrame = NULL;
	XMFLOAT3					m_xmf3Velocity;
	XMFLOAT3					now;
	float						velocity;
	float						ScaleSize;
protected:
	LPVOID						m_pUpdatedContext;
public:
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }
	virtual void OnInitialize();
	virtual void CommandF4(CGameObject* m_pPlayer);
	virtual void Move(const XMFLOAT3& xmf3Shift);
	virtual void Update(CGameObject* m_pPlayer, float fTimeElapsed);
	virtual void SetUpdatedContext(LPVOID pContext) { m_pUpdatedContext = pContext; }
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent = NULL);
	virtual void OnUpdateCallback();
};

class CApacheObject : public CHellicopterObject
{
public:
	CApacheObject(void* pContext = NULL);
	virtual ~CApacheObject();
public:
	virtual void OnInitialize();
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent = NULL);
};

class CGunshipObject : public CHellicopterObject
{
public:
	CGunshipObject(void* pContext = NULL);
	virtual ~CGunshipObject();
public:
	virtual void OnInitialize();
};

class CSuperCobraObject : public CHellicopterObject
{
public:
	CSuperCobraObject(void* pContext = NULL);
	virtual ~CSuperCobraObject();
public:
	virtual void OnInitialize();
};

class CMi24Object : public CHellicopterObject
{
public:
	CMi24Object(void* pContext = NULL);
	virtual ~CMi24Object();
public:
	virtual void OnInitialize();
};

class MyGameObject
{
protected:
	MyMesh **m_ppMeshes = NULL;
	int m_nMeshes = 0;
	XMFLOAT4X4 m_xmf4x4World;
	CShader* m_pShader = NULL;
public:
	MyGameObject(int nMeshes = 1);
	virtual ~MyGameObject();

	virtual void OnPrepareRender() { }
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) { }
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	virtual void ReleaseShaderVariables() { }
	virtual void ReleaseUploadBuffers();
	virtual void SetMesh(int nIndex, MyMesh* pMesh);
	virtual void SetShader(CShader* pShader);
};

class CHeightMapTerrain : public MyGameObject
{
public:
	CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int
		nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4
		xmf4Color);
	virtual ~CHeightMapTerrain();
private:
	//?????? ???? ?????? ?????? ??????????. 
	CHeightMapImage* m_pHeightMapImage;
	//???? ???? ?????? ???? ????????. 
	int m_nWidth;
	int m_nLength;
	//?????? ?????? ?? ?? ?????? ???????? ???????? ?????? ????????. 
	XMFLOAT3 m_xmf3Scale;
public:
	//?????? ?????? ???????? ????????(???? ??????). ???? ???? ?????? ???????? y?? ???? ??????. 
	float GetHeight(float x, float z) { return(m_pHeightMapImage->GetHeight(x / m_xmf3Scale.x, z / m_xmf3Scale.z) * m_xmf3Scale.y); }
	//?????? ???? ?????? ???????? ????????(???? ??????). ???? ???? ???? ?????? ????????. 
	XMFLOAT3 GetNormal(float x, float z) {
		return(m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x), int(z / m_xmf3Scale.z)));
	}
	int GetHeightMapWidth() { return(m_pHeightMapImage->GetHeightMapWidth()); }
	int GetHeightMapLength() { return(m_pHeightMapImage->GetHeightMapLength()); }
	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	//?????? ????(????/????)?? ????????. ???? ???? ?????? ???????? ???? ??????. 
	float GetWidth() { return(m_nWidth * m_xmf3Scale.x); }
	float GetLength() { return(m_nLength * m_xmf3Scale.z); }
};
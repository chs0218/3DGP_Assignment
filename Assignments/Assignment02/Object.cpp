//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"
#include "Shader.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMaterial::CMaterial()
{
}

CMaterial::~CMaterial()
{
}

void CMaterial::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CMaterial::ReleaseShaderVariables()
{
}

void CMaterial::ReleaseUploadBuffers()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGameObject::CGameObject(UINT nMaterials)
{
	m_nMaterials = nMaterials;
	m_ppMaterials = new CMaterial*[m_nMaterials];
	for (UINT i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;

	m_xmf4x4World = Matrix4x4::Identity();
}
 
CGameObject::~CGameObject()
{
	ReleaseShaderVariables();

	if (m_pMesh) m_pMesh->Release();
	m_pMesh = NULL;

	if (m_ppMaterials)
	{
		for (UINT i = 0; i < m_nMaterials; i++) if (m_ppMaterials[i]) m_ppMaterials[i]->Release();
		delete[] m_ppMaterials;
	}

	if (m_pShader) m_pShader->Release();
}

void CGameObject::SetMesh(CMesh *pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (pMesh) pMesh->AddRef();
}

void CGameObject::SetShader(CShader *pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CGameObject::SetAlbedoColor(UINT nIndex, XMFLOAT4 xmf4Color)
{
	if ((nIndex >= 0) && (nIndex < m_nMaterials))
	{
		if (!m_ppMaterials[nIndex])
		{
			m_ppMaterials[nIndex] = new CMaterial();
			m_ppMaterials[nIndex]->AddRef();
		}
		m_ppMaterials[nIndex]->SetAlbedoColor(xmf4Color);
	}
}

void CGameObject::SetEmissionColor(UINT nIndex, XMFLOAT4 xmf4Color)
{
	if ((nIndex >= 0) && (nIndex < m_nMaterials))
	{
		if (!m_ppMaterials[nIndex])
		{
			m_ppMaterials[nIndex] = new CMaterial();
			m_ppMaterials[nIndex]->AddRef();
		}
		m_ppMaterials[nIndex]->SetEmissionColor(xmf4Color);
	}
}

void CGameObject::SetMaterial(UINT nIndex, CMaterial *pMaterial)
{
	if ((nIndex >= 0) && (nIndex < m_nMaterials))
	{
		if (m_ppMaterials[nIndex]) m_ppMaterials[nIndex]->Release();
		m_ppMaterials[nIndex] = pMaterial;
		if (pMaterial) pMaterial->AddRef();
	}
}

void CGameObject::SetMaterial(UINT nIndex, UINT nReflection)
{
	if ((nIndex >= 0) && (nIndex < m_nMaterials))
	{
		if (!m_ppMaterials[nIndex])
		{
			m_ppMaterials[nIndex] = new CMaterial();
			m_ppMaterials[nIndex]->AddRef();
		}
		m_ppMaterials[nIndex]->m_nMaterial = nReflection;
	}
}

void CGameObject::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbGameObject = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbGameObject->Map(0, NULL, (void **)&m_pcbMappedGameObject);
}

void CGameObject::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObject)
	{
		m_pd3dcbGameObject->Unmap(0, NULL);
		m_pd3dcbGameObject->Release();
	}
	for (UINT i = 0; i < m_nMaterials; i++) if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseShaderVariables();
	if (m_pShader) m_pShader->ReleaseShaderVariables();
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMStoreFloat4x4(&m_pcbMappedGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbGameObject->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(1, d3dGpuVirtualAddress);
}

void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	OnPrepareRender();

	if (m_pMesh && m_ppMaterials)
	{
		if (m_pShader)
		{
			m_pShader->Render(pd3dCommandList, pCamera);
			m_pShader->UpdateShaderVariables(pd3dCommandList);
		}
		if (m_pd3dcbGameObject && m_pcbMappedGameObject) UpdateShaderVariables(pd3dCommandList);

		m_pMesh->OnPreRender(pd3dCommandList);
		for (UINT i = 0; i < m_nMaterials; i++)
		{
			if (m_ppMaterials[i])
			{
				pd3dCommandList->SetGraphicsRoot32BitConstant(4, m_ppMaterials[i]->m_nMaterial, 0);
				pd3dCommandList->SetGraphicsRoot32BitConstants(4, 4, &m_ppMaterials[i]->m_xmf4AlbedoColor, 4);
				pd3dCommandList->SetGraphicsRoot32BitConstants(4, 4, &m_ppMaterials[i]->m_xmf4EmissionColor, 8);
			}
			m_pMesh->Render(pd3dCommandList, i);
		}
	}
}

void CGameObject::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();
	for (UINT i = 0; i < m_nMaterials; i++) if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseUploadBuffers();
	if (m_pShader) m_pShader->ReleaseUploadBuffers();
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}

void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::Rotate(XMFLOAT3 *pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::LoadGameObjectFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName)
{
	FILE *pFile = NULL;
	::fopen_s(&pFile, pstrFileName, "rb");
	::rewind(pFile);

	char pstrToken[64] = { '\0' };
	char pstrGameObjectName[64] = { '\0' };
	char pstrFilePath[64] = { '\0' };

	BYTE nStrLength = 0, nObjectNameLength = 0;
	UINT nReads = 0, nMaterials = 0;
	size_t nConverted = 0;

	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pFile); //"<GameObject>:"
	nReads = (UINT)::fread(&nObjectNameLength, sizeof(BYTE), 1, pFile);
	nReads = (UINT)::fread(pstrGameObjectName, sizeof(char), nObjectNameLength, pFile);
	pstrGameObjectName[nObjectNameLength] = '\0';

	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pFile); //"<Materials>:"
	nReads = (UINT)::fread(&nMaterials, sizeof(int), 1, pFile);

	strcpy_s(m_pstrName, 64, pstrGameObjectName);

	XMFLOAT4 xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), xmf4EmissionColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	for (UINT k = 0; k < nMaterials; k++)
	{
		SetMaterial(k, 1);

		nReads = (UINT)::fread(&xmf4AlbedoColor, sizeof(float), 4, pFile);
		SetAlbedoColor(k, xmf4AlbedoColor);

		nReads = (UINT)::fread(&xmf4EmissionColor, sizeof(float), 4, pFile);
		SetEmissionColor(k, xmf4EmissionColor);
	}

	nReads = (UINT)::fread(&m_xmf4x4World, sizeof(float), 16, pFile);

	strcpy_s(pstrFilePath, 64, "Models/");
	strcpy_s(pstrFilePath + 7, 64 - 7, pstrGameObjectName);
	strcpy_s(pstrFilePath + 7 + nObjectNameLength, 64 - 7 - nObjectNameLength, ".bin");
	CMesh *pMesh = new CMesh(pd3dDevice, pd3dCommandList, pstrFilePath);
	SetMesh(pMesh);

	::fclose(pFile);
}

//==========================================================================================
MyGameObject::MyGameObject(int nMeshes)
{
	m_xmf4x4World = Matrix4x4::Identity();
	m_nMeshes = nMeshes;
	m_ppMeshes = NULL;
	if (m_nMeshes > 0)
	{
		m_ppMeshes = new CMesh * [m_nMeshes];
		for (int i = 0; i < m_nMeshes; i++) m_ppMeshes[i] = NULL;
	}
}

MyGameObject::~MyGameObject()
{
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->Release();
			m_ppMeshes[i] = NULL;
		}
		delete[] m_ppMeshes;
	}
	if (m_pShader)
	{
		m_pShader->ReleaseShaderVariables();
		m_pShader->Release();
	}
}

void MyGameObject::SetMesh(int nIndex, CMesh* pMesh)
{
	if (m_ppMeshes)
	{
		if (m_ppMeshes[nIndex]) m_ppMeshes[nIndex]->Release();
		m_ppMeshes[nIndex] = pMesh;
		if (pMesh) pMesh->AddRef();
	}
}
void MyGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
	//객체의 월드 변환 행렬을 루트 상수(32-비트 값)를 통하여 셰이더 변수(상수 버퍼)로 복사한다. 
	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 16, &xmf4x4World, 0);
}

void MyGameObject::OnPrepareRender()
{
}
void MyGameObject::SetShader(CShader* pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}
void MyGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();
	UpdateShaderVariables(pd3dCommandList);
	if (m_pShader) m_pShader->Render(pd3dCommandList, pCamera);
	

	//게임 객체가 포함하는 모든 메쉬를 렌더링한다. 
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->Render(pd3dCommandList);
		}
	}
}

CHeightMapTerrain::CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName,
	int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color) : MyGameObject(0)
{
	//지형에 사용할 높이 맵의 가로, 세로의 크기이다. 
	m_nWidth = nWidth;
	m_nLength = nLength;
	/*지형 객체는 격자 메쉬들의 배열로 만들 것이다. nBlockWidth, nBlockLength는 격자 메쉬 하나의 가로, 세로 크
	기이다. cxQuadsPerBlock, czQuadsPerBlock은 격자 메쉬의 가로 방향과 세로 방향 사각형의 개수이다.*/
	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;
	//xmf3Scale는 지형을 실제로 몇 배 확대할 것인가를 나타낸다.
	m_xmf3Scale = xmf3Scale;
	//지형에 사용할 높이 맵을 생성한다. 
	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);
	//지형에서 가로 방향, 세로 방향으로 격자 메쉬가 몇 개가 있는 가를 나타낸다. 
	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;
	//지형 전체를 표현하기 위한 격자 메쉬의 개수이다. 
	m_nMeshes = cxBlocks * czBlocks;
	//지형 전체를 표현하기 위한 격자 메쉬에 대한 포인터 배열을 생성한다. 
	m_ppMeshes = new CMesh * [m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)m_ppMeshes[i] = NULL;
	CHeightMapGridMesh* pHeightMapGridMesh = NULL;
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			//지형의 일부분을 나타내는 격자 메쉬의 시작 위치(좌표)이다. 
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			//지형의 일부분을 나타내는 격자 메쉬를 생성하여 지형 메쉬에 저장한다. 
			pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, xStart,
				zStart, nBlockWidth, nBlockLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
			SetMesh(x + (z * cxBlocks), pHeightMapGridMesh);
		}
	}
	//지형을 렌더링하기 위한 셰이더를 생성한다. 
	CTerrainShader* pShader = new CTerrainShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	SetShader(pShader);
}
CHeightMapTerrain::~CHeightMapTerrain(void)
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}
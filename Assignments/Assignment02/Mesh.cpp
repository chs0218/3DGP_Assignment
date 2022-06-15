//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Mesh.h"

CMesh::CMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName)
{
	if (pstrFileName) LoadMeshFromFile(pd3dDevice, pd3dCommandList, pstrFileName);
}

CMesh::~CMesh()
{
	if (m_pxmf3Positions) delete[] m_pxmf3Positions;
	if (m_pxmf3Normals) delete[] m_pxmf3Normals;

	if (m_pnIndices) delete[] m_pnIndices;

	if (m_pnSubSetIndices) delete[] m_pnSubSetIndices;
	if (m_pnSubSetStartIndices) delete[] m_pnSubSetStartIndices;
	for (UINT i = 0; i < m_nSubsets; i++) if (m_ppnSubSetIndices[i]) delete[] m_ppnSubSetIndices[i];
	if (m_ppnSubSetIndices) delete[] m_ppnSubSetIndices;

	if (m_pd3dVertexBufferViews) delete[] m_pd3dVertexBufferViews;

	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();
	if (m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release();

	for (UINT i = 0; i < m_nSubsets; i++) if (m_ppd3dIndexBuffers[i]) m_ppd3dIndexBuffers[i]->Release();
	if (m_ppd3dIndexBuffers) delete[] m_ppd3dIndexBuffers;

	if (m_pd3dIndexBufferViews) delete[] m_pd3dIndexBufferViews;
}

void CMesh::ReleaseUploadBuffers()
{
	if (m_pd3dPositionUploadBuffer) m_pd3dPositionUploadBuffer->Release();
	if (m_pd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	if (m_ppd3dIndexUploadBuffers)
	{
		for (UINT i = 0; i < m_nSubsets; i++) if (m_ppd3dIndexUploadBuffers[i]) m_ppd3dIndexUploadBuffers[i]->Release();
		delete[] m_ppd3dIndexUploadBuffers;
	}

	m_pd3dPositionUploadBuffer = NULL;
	m_pd3dNormalUploadBuffer = NULL;
	m_ppd3dIndexUploadBuffers = NULL;
};

void CMesh::OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, m_nVertexBufferViews, m_pd3dVertexBufferViews);
}

void CMesh::Render(ID3D12GraphicsCommandList *pd3dCommandList, UINT nSubset)
{
	if (m_nSubsets > 0)
	{
		pd3dCommandList->IASetIndexBuffer(&m_pd3dIndexBufferViews[nSubset]);
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubset], 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

void CMesh::LoadMeshFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName)
{
#ifdef _WITH_TEXT_MESH
	ifstream InFile(pstrFileName);

	char pstrToken[64] = { '\0' };

	for ( ; ; )
	{
		InFile >> pstrToken;
		if (!InFile) break;

		if (!strcmp(pstrToken, "<Vertices>:"))
		{
			InFile >> m_nVertices;
			m_pxmf3Positions = new XMFLOAT3[m_nVertices];
			for (UINT i = 0; i < m_nVertices; i++) InFile >> m_pxmf3Positions[i].x >> m_pxmf3Positions[i].y >> m_pxmf3Positions[i].z;
		}
		else if (!strcmp(pstrToken, "<Normals>:"))
		{
			InFile >> pstrToken;
			m_pxmf3Normals = new XMFLOAT3[m_nVertices];
			for (UINT i = 0; i < m_nVertices; i++) InFile >> m_pxmf3Normals[i].x >> m_pxmf3Normals[i].y >> m_pxmf3Normals[i].z;
		}
		else if (!strcmp(pstrToken, "<Indices>:"))
		{
			InFile >> m_nIndices;
			m_pnIndices = new UINT[m_nIndices];
			for (UINT i = 0; i < m_nIndices; i++) InFile >> m_pnIndices[i];
		}
	}
#else
	FILE *pFile = NULL;
	::fopen_s(&pFile, pstrFileName, "rb");
	::rewind(pFile);

	char pstrToken[64] = { '\0' };

	BYTE nStrLength = 0;
	UINT nReads = 0;

	while (!::feof(pFile))
	{
		nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
		if (nReads == 0) break;
		nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pFile);
		pstrToken[nStrLength] = '\0';

		if (!strcmp(pstrToken, "<BoundingBox>:"))
		{
			nReads = (UINT)::fread(&m_xmBoundingBox.Center, sizeof(float), 3, pFile);
			nReads = (UINT)::fread(&m_xmBoundingBox.Extents, sizeof(float), 3, pFile);
		}
		else if (!strcmp(pstrToken, "<Vertices>:"))
		{
			nReads = (UINT)::fread(&m_nVertices, sizeof(int), 1, pFile);
			m_pxmf3Positions = new XMFLOAT3[m_nVertices];
			nReads = (UINT)::fread(m_pxmf3Positions, sizeof(float), 3 * m_nVertices, pFile);
		}
		else if (!strcmp(pstrToken, "<Normals>:"))
		{
			nReads = (UINT)::fread(&m_nVertices, sizeof(int), 1, pFile);
			m_pxmf3Normals = new XMFLOAT3[m_nVertices];
			nReads = (UINT)::fread(m_pxmf3Normals, sizeof(float), 3 * m_nVertices, pFile);
		}
		else if (!strcmp(pstrToken, "<Indices>:"))
		{
			nReads = (UINT)::fread(&m_nIndices, sizeof(int), 1, pFile);
			m_pnIndices = new UINT[m_nIndices];
			nReads = (UINT)::fread(m_pnIndices, sizeof(UINT), m_nIndices, pFile);
		}
		else if (!strcmp(pstrToken, "<SubMeshes>:"))
		{
			nReads = (UINT)::fread(&m_nSubsets, sizeof(int), 1, pFile);

			m_pnSubSetIndices = new UINT[m_nSubsets];
			m_pnSubSetStartIndices = new UINT[m_nSubsets];
			m_ppnSubSetIndices = new UINT*[m_nSubsets];

			for (UINT i = 0; i < m_nSubsets; i++)
			{
				nReads = (UINT)::fread(&m_pnSubSetStartIndices[i], sizeof(UINT), 1, pFile);
				nReads = (UINT)::fread(&m_pnSubSetIndices[i], sizeof(UINT), 1, pFile);
				nReads = (UINT)::fread(&m_nIndices, sizeof(int), 1, pFile);
				m_ppnSubSetIndices[i] = new UINT[m_pnSubSetIndices[i]];
				nReads = (UINT)::fread(m_ppnSubSetIndices[i], sizeof(UINT), m_pnSubSetIndices[i], pFile);
			}

			break;
		}
	}

	::fclose(pFile);
#endif

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);
	m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);

	m_nVertexBufferViews = 2;
	m_pd3dVertexBufferViews = new D3D12_VERTEX_BUFFER_VIEW[m_nVertexBufferViews];

	m_pd3dVertexBufferViews[0].BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_pd3dVertexBufferViews[0].StrideInBytes = sizeof(XMFLOAT3);
	m_pd3dVertexBufferViews[0].SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dVertexBufferViews[1].BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
	m_pd3dVertexBufferViews[1].StrideInBytes = sizeof(XMFLOAT3);
	m_pd3dVertexBufferViews[1].SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_ppd3dIndexBuffers = new ID3D12Resource*[m_nSubsets];
	m_ppd3dIndexUploadBuffers = new ID3D12Resource*[m_nSubsets];
	m_pd3dIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubsets];

	for (UINT i = 0; i < m_nSubsets; i++)
	{
		m_ppd3dIndexBuffers[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[i], sizeof(UINT) * m_pnSubSetIndices[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dIndexUploadBuffers[i]);

		m_pd3dIndexBufferViews[i].BufferLocation = m_ppd3dIndexBuffers[i]->GetGPUVirtualAddress();
		m_pd3dIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
		m_pd3dIndexBufferViews[i].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[i];
	}
}

//================================================================================
CHeightMapImage::CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;
	BYTE* pHeightMapPixels = new BYTE[m_nWidth * m_nLength];
	//������ ���� �д´�. ���� �� �̹����� ���� ����� ���� RAW �̹����̴�. 
	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	DWORD dwBytesRead;
	::ReadFile(hFile, pHeightMapPixels, (m_nWidth * m_nLength), &dwBytesRead, NULL);
	::CloseHandle(hFile);
	m_pHeightMapPixels = new BYTE[m_nWidth * m_nLength];
	for (int y = 0; y < m_nLength; y++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			m_pHeightMapPixels[x + ((m_nLength - 1 - y) * m_nWidth)] = pHeightMapPixels[x + (y * m_nWidth)];
		}
	}
	if (pHeightMapPixels) delete[] pHeightMapPixels;
}

CHeightMapImage::~CHeightMapImage()
{
	if (m_pHeightMapPixels) delete[] m_pHeightMapPixels;
	m_pHeightMapPixels = NULL;
}

XMFLOAT3 CHeightMapImage::GetHeightMapNormal(int x, int z)
{
	//x-��ǥ�� z-��ǥ�� ���� ���� ������ ����� ������ ���� ���ʹ� y-�� ���� �����̴�.
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_nWidth) || (z >= m_nLength))
		return(XMFLOAT3(0.0f, 1.0f, 0.0f));
	/*���� �ʿ��� (x, z) ��ǥ�� �ȼ� ���� ������ �� ���� �� (x+1, z), (z, z+1)�� ���� �ȼ� ���� ����Ͽ� ���� ���͸�
	����Ѵ�.*/
	int nHeightMapIndex = x + (z * m_nWidth);
	int xHeightMapAdd = (x < (m_nWidth - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -m_nWidth;
	//(x, z), (x+1, z), (z, z+1)�� �ȼ����� ������ ���̸� ���Ѵ�. 
	float y1 = (float)m_pHeightMapPixels[nHeightMapIndex] * m_xmf3Scale.y;
	float y2 = (float)m_pHeightMapPixels[nHeightMapIndex + xHeightMapAdd] * m_xmf3Scale.y;
	float y3 = (float)m_pHeightMapPixels[nHeightMapIndex + zHeightMapAdd] * m_xmf3Scale.y;
	//xmf3Edge1�� (0, y3, m_xmf3Scale.z) - (0, y1, 0) �����̴�. 
	XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.0f, y3 - y1, m_xmf3Scale.z);
	//xmf3Edge2�� (m_xmf3Scale.x, y2, 0) - (0, y1, 0) �����̴�. 
	XMFLOAT3 xmf3Edge2 = XMFLOAT3(m_xmf3Scale.x, y2 - y1, 0.0f);
	//���� ���ʹ� xmf3Edge1�� xmf3Edge2�� ������ ����ȭ�ϸ� �ȴ�. 
	XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge1, xmf3Edge2, true);
	return(xmf3Normal);
}

#define _WITH_APPROXIMATE_OPPOSITE_CORNER
float CHeightMapImage::GetHeight(float fx, float fz)
{
	/*������ ��ǥ (fx, fz)�� �̹��� ��ǥ���̴�. ���� ���� x-��ǥ�� z-��ǥ�� ���� ���� ������ ����� ������ ���̴�
	0�̴�.*/
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) return(0.0f);
	//���� ���� ��ǥ�� ���� �κа� �Ҽ� �κ��� ����Ѵ�. 
	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;
	float fBottomLeft = (float)m_pHeightMapPixels[x + (z * m_nWidth)];
	float fBottomRight = (float)m_pHeightMapPixels[(x + 1) + (z * m_nWidth)];
	float fTopLeft = (float)m_pHeightMapPixels[x + ((z + 1) * m_nWidth)];
	float fTopRight = (float)m_pHeightMapPixels[(x + 1) + ((z + 1) * m_nWidth)];
#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	//z-��ǥ�� 1, 3, 5, ...�� ��� �ε����� �����ʿ��� �������� �����ȴ�. 
	bool bRightToLeft = ((z % 2) != 0);
	if (bRightToLeft)
	{
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
#endif
	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;
	return(fHeight);
}

CHeightMapGridMesh::CHeightMapGridMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, void* pContext) : CMesh(pd3dDevice, pd3dCommandList)
{
	//������ ����(����)�� ������ (nWidth * nLength)�̴�. 
	m_nVertices = nWidth * nLength;
	m_nStride = sizeof(CDiffusedVertex);
	//���ڴ� �ﰢ�� ��Ʈ������ �����Ѵ�. 
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;
	CDiffusedVertex* pVertices = new CDiffusedVertex[m_nVertices];
	/*xStart�� zStart�� ������ ���� ��ġ(x-��ǥ�� z-��ǥ)�� ��Ÿ����. Ŀ�ٶ� ������ ���ڵ��� ������ �迭�� ���� ��
	�䰡 �ֱ� ������ ��ü �������� �� ������ ���� ��ġ�� ��Ÿ���� ������ �ʿ��ϴ�.*/
	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;
	for (int i = 0, z = zStart; z < (zStart + nLength); z++)
	{
		for (int x = xStart; x < (xStart + nWidth); x++, i++)
		{
			//������ ���̿� ������ ���� �����κ��� ���Ѵ�. 
			XMFLOAT3 xmf3Position = XMFLOAT3((x * m_xmf3Scale.x), OnGetHeight(x, z, pContext), (z * m_xmf3Scale.z));
			XMFLOAT4 xmf3Color = Vector4::Add(OnGetColor(x, z, pContext), xmf4Color);
			pVertices[i] = CDiffusedVertex(xmf3Position, xmf3Color);
			if (fHeight < fMinHeight) fMinHeight = fHeight;
			if (fHeight > fMaxHeight) fMaxHeight = fHeight;
		}
	}
	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices,
		m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
	delete[] pVertices;
	m_nIndices = ((nWidth * 2) * (nLength - 1)) + ((nLength - 1) - 1);
	UINT* pnIndices = new UINT[m_nIndices];
	for (int j = 0, z = 0; z < nLength - 1; z++)
	{
		if ((z % 2) == 0)
		{
			//Ȧ�� ��° ���̹Ƿ�(z = 0, 2, 4, ...) �ε����� ���� ������ ���ʿ��� ������ �����̴�. 
			for (int x = 0; x < nWidth; x++)
			{
				//ù ��° ���� �����ϰ� ���� �ٲ� ������(x == 0) ù ��° �ε����� �߰��Ѵ�. 
				if ((x == 0) && (z > 0)) pnIndices[j++] = (UINT)(x + (z * nWidth));
				//�Ʒ�(x, z), ��(x, z+1)�� ������ �ε����� �߰��Ѵ�. 
				pnIndices[j++] = (UINT)(x + (z * nWidth));
				pnIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
		else
		{
			//¦�� ��° ���̹Ƿ�(z = 1, 3, 5, ...) �ε����� ���� ������ �����ʿ��� ���� �����̴�. 
			for (int x = nWidth - 1; x >= 0; x--)
			{
				//���� �ٲ� ������(x == (nWidth-1)) ù ��° �ε����� �߰��Ѵ�. 
				if (x == (nWidth - 1)) pnIndices[j++] = (UINT)(x + (z * nWidth));
				//�Ʒ�(x, z), ��(x, z+1)�� ������ �ε����� �߰��Ѵ�. 
				pnIndices[j++] = (UINT)(x + (z * nWidth));
				pnIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
	}
	m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pnIndices,
		sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER,
		&m_pd3dIndexUploadBuffer);
	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;
	delete[] pnIndices;
}

CHeightMapGridMesh::~CHeightMapGridMesh()
{
}

//���� �� �̹����� �ȼ� ���� ������ ���̷� ��ȯ�Ѵ�. 
float CHeightMapGridMesh::OnGetHeight(int x, int z, void* pContext)
{
	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
	BYTE* pHeightMapPixels = pHeightMapImage->GetHeightMapPixels();
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	int nWidth = pHeightMapImage->GetHeightMapWidth();
	float fHeight = pHeightMapPixels[x + (z * nWidth)] * xmf3Scale.y;
	return(fHeight);
}

XMFLOAT4 CHeightMapGridMesh::OnGetColor(int x, int z, void* pContext)
{
	//������ ���� ����(�������� ���������� ����)�̴�. 
	XMFLOAT3 xmf3LightDirection = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	xmf3LightDirection = Vector3::Normalize(xmf3LightDirection);
	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	//������ ����(����, ���)�̴�. 
	XMFLOAT4 xmf4IncidentLightColor(0.9f, 0.8f, 0.4f, 1.0f);
	/*���� (x, z)���� ������ �ݻ�Ǵ� ��(����)�� ���� (x, z)�� ���� ���Ϳ� ������ ���� ������ ����(cos)�� ������ 3��
	�� ���� (x+1, z), (x, z+1), (x+1, z+1)�� ���� ���Ϳ� ������ ���� ������ ������ ����Ͽ� ���Ѵ�. ���� (x, z)�� ��
	���� ���� ����(����)�� �ݻ�Ǵ� ��(����)�� ���� ���̴�.*/
	float fScale = Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z),
		xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z),
		xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z + 1),
		xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z + 1),
		xmf3LightDirection);
	fScale = (fScale / 4.0f) + 0.05f;
	if (fScale > 1.0f) fScale = 1.0f;
	if (fScale < 0.25f) fScale = 0.25f;
	//fScale�� ���� ����(���)�� �ݻ�Ǵ� �����̴�. 
	XMFLOAT4 xmf4Color = Vector4::Multiply(fScale, xmf4IncidentLightColor);
	return(xmf4Color);
}

void CMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dVertexBufferView);
	if (m_pd3dIndexBuffer)
	{
		pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
		pd3dCommandList->DrawIndexedInstanced(m_nIndices, 1, 0, 0, 0);
		//�ε��� ���۰� ������ �ε��� ���۸� ����������(IA: �Է� ������)�� �����ϰ� �ε����� ����Ͽ� �������Ѵ�. 
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}
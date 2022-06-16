//------------------------------------------------------- ----------------------
// File: Mesh.h
//-----------------------------------------------------------------------------

#pragma once
class CHeightMapImage
{
private:
	//���� �� �̹��� �ȼ�(8-��Ʈ)���� ������ �迭�̴�. �� �ȼ��� 0~255�� ���� ���´�. 
	BYTE* m_pHeightMapPixels;
	//���� �� �̹����� ���ο� ���� ũ���̴�. 
	int m_nWidth;
	int m_nLength;
	//���� �� �̹����� ������ �� �� Ȯ���Ͽ� ����� ���ΰ��� ��Ÿ���� ������ �����̴�. 
	XMFLOAT3 m_xmf3Scale;
public:
	CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale);
	~CHeightMapImage(void);
	//���� �� �̹������� (x, z) ��ġ�� �ȼ� ���� ����� ������ ���̸� ��ȯ�Ѵ�. 
	float GetHeight(float x, float z);
	//���� �� �̹������� (x, z) ��ġ�� ���� ���͸� ��ȯ�Ѵ�. 
	XMFLOAT3 GetHeightMapNormal(int x, int z);
	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	BYTE* GetHeightMapPixels() { return(m_pHeightMapPixels); }
	int GetHeightMapWidth() { return(m_nWidth); }
	int GetHeightMapLength() { return(m_nLength); }
};

//������ ǥ���ϱ� ���� Ŭ������ �����Ѵ�. 
class CVertex
{
protected:
	//������ ��ġ �����̴�(��� ������ �ּ��� ��ġ ���͸� ������ �Ѵ�). 
	XMFLOAT3 m_xmf3Position;
public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CVertex(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	~CVertex() { }
};

class CDiffusedVertex : public CVertex
{
protected:
	//������ �����̴�. 
	XMFLOAT4 m_xmf4Diffuse;
public:
	CDiffusedVertex() {
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); m_xmf4Diffuse =
			XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	CDiffusedVertex(float x, float y, float z, XMFLOAT4 xmf4Diffuse) {
		m_xmf3Position =
			XMFLOAT3(x, y, z); m_xmf4Diffuse = xmf4Diffuse;
	}
	CDiffusedVertex(XMFLOAT3 xmf3Position, XMFLOAT4 xmf4Diffuse) {
		m_xmf3Position =
			xmf3Position; m_xmf4Diffuse = xmf4Diffuse;
	}
	~CDiffusedVertex() { }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CMesh
{
public:
	CMesh() { }
	CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName = NULL);
    virtual ~CMesh() { }

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	virtual void ReleaseUploadBuffers() { }

protected:
	D3D12_PRIMITIVE_TOPOLOGY		m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT							m_nSlot = 0;
	UINT							m_nVertices = 0;
	UINT							m_nOffset = 0;

	UINT							m_nType = 0;

public:
	UINT GetType() { return(m_nType); }
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList) { }
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet) { }
protected:
	UINT							m_nStride = 0;
	BoundingBox						m_xmBoundingBox;
	XMFLOAT3* m_pxmf3Positions = NULL;
	ID3D12Resource* m_pd3dPositionBuffer = NULL;
	ID3D12Resource* m_pd3dPositionUploadBuffer = NULL;

	XMFLOAT3* m_pxmf3Normals = NULL;
	ID3D12Resource* m_pd3dNormalBuffer = NULL;
	ID3D12Resource* m_pd3dNormalUploadBuffer = NULL;

	UINT							m_nVertexBufferViews = 0;
	D3D12_VERTEX_BUFFER_VIEW* m_pd3dVertexBufferViews = NULL;

	UINT							m_nIndices = 0;
	UINT* m_pnIndices = NULL;

	UINT* m_pnSubSetIndices = NULL;
	UINT* m_pnSubSetStartIndices = NULL;
	UINT** m_ppnSubSetIndices = NULL;
	
	ID3D12Resource** m_ppd3dIndexBuffers = NULL;
	ID3D12Resource** m_ppd3dIndexUploadBuffers = NULL;

	D3D12_INDEX_BUFFER_VIEW* m_pd3dIndexBufferViews = NULL;
protected:
	ID3D12Resource* m_pd3dVertexBuffer = NULL;
	ID3D12Resource* m_pd3dVertexUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView;

	ID3D12Resource* m_pd3dIndexBuffer = NULL;
	ID3D12Resource* m_pd3dIndexUploadBuffer = NULL;
	D3D12_INDEX_BUFFER_VIEW m_d3dIndexBufferView;
public:
	UINT							m_nSubsets = 0;
	void LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define VERTEXT_POSITION			0x01
#define VERTEXT_COLOR				0x02
#define VERTEXT_NORMAL				0x04

class CMeshLoadInfo
{
public:
	CMeshLoadInfo() { }
	~CMeshLoadInfo();

public:
	char							m_pstrMeshName[256] = { 0 };

	UINT							m_nType = 0x00;

	XMFLOAT3						m_xmf3AABBCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3						m_xmf3AABBExtents = XMFLOAT3(0.0f, 0.0f, 0.0f);

	int								m_nVertices = 0;
	XMFLOAT3						*m_pxmf3Positions = NULL;
	XMFLOAT4						*m_pxmf4Colors = NULL;
	XMFLOAT3						*m_pxmf3Normals = NULL;

	int								m_nIndices = 0;
	UINT							*m_pnIndices = NULL;

	int								m_nSubMeshes = 0;
	int								*m_pnSubSetIndices = NULL;
	UINT							**m_ppnSubSetIndices = NULL;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CMeshFromFile : public CMesh
{
public:
	CMeshFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CMeshLoadInfo *pMeshInfo);
	virtual ~CMeshFromFile();

public:
	virtual void ReleaseUploadBuffers();

protected:
	ID3D12Resource					*m_pd3dPositionBuffer = NULL;
	ID3D12Resource					*m_pd3dPositionUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dPositionBufferView;

	int								m_nSubMeshes = 0;
	int								*m_pnSubSetIndices = NULL;

	ID3D12Resource					**m_ppd3dSubSetIndexBuffers = NULL;
	ID3D12Resource					**m_ppd3dSubSetIndexUploadBuffers = NULL;
	D3D12_INDEX_BUFFER_VIEW			*m_pd3dSubSetIndexBufferViews = NULL;

public:
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CMeshIlluminatedFromFile : public CMeshFromFile
{
public:
	CMeshIlluminatedFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CMeshLoadInfo *pMeshInfo);
	virtual ~CMeshIlluminatedFromFile();

	virtual void ReleaseUploadBuffers();

protected:
	ID3D12Resource					*m_pd3dNormalBuffer = NULL;
	ID3D12Resource					*m_pd3dNormalUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dNormalBufferView;

public:
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet);
};

class CHeightMapGridMesh : public CMesh
{
protected:
	//������ ũ��(����: x-����, ����: z-����)�̴�. 
	int m_nWidth;
	int m_nLength;
	/*������ ������(����: x-����, ����: z-����, ����: y-����) �����̴�. ���� ���� �޽��� �� ������ x-��ǥ, y-��ǥ, z-��ǥ�� ������ ������ x-��ǥ, y-��ǥ, z-��ǥ�� ���� ���� ���´�. ��, ���� ������ x-�� ������ ������ 1�� �ƴ�
	�� ������ ������ x-��ǥ�� �ȴ�. �̷��� �ϸ� ���� ����(���� ����)�� ����ϴ��� ū ũ���� ����(����)�� ������
	�� �ִ�.*/
	XMFLOAT3 m_xmf3Scale;
public:
	CHeightMapGridMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale =
		XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f), void
		* pContext = NULL);
	virtual ~CHeightMapGridMesh();
	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	int GetWidth() { return(m_nWidth); }
	int GetLength() { return(m_nLength); }
	//������ ��ǥ�� (x, z)�� �� ����(����)�� ���̸� ��ȯ�ϴ� �Լ��̴�. 
	virtual float OnGetHeight(int x, int z, void* pContext);
	//������ ��ǥ�� (x, z)�� �� ����(����)�� ������ ��ȯ�ϴ� �Լ��̴�. 
	virtual XMFLOAT4 OnGetColor(int x, int z, void* pContext);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);
};
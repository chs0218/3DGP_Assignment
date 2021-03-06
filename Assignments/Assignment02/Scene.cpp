//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"

std::random_device rd{};
std::default_random_engine dre{ rd() };

CScene::CScene()
{
}

CScene::~CScene()
{
}

void CScene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 2;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);
	m_pLights[0].m_bEnable = true;
	m_pLights[0].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
	m_pLights[0].m_xmf3Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_pLights[1].m_bEnable = true;
	m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights[1].m_fRange = 1000.0f;
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[1].m_fFalloff = 8.0f;
	m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(60.0f));
	m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(40.0f));
}

void CScene::SpawnEnemy()
{
	std::vector<CGameObject*>::iterator index = std::find_if(v_GameObjects.begin(), v_GameObjects.end(), [](const CGameObject* target) { return !target->isEnable; });
	if (index != v_GameObjects.end())
	{
		std::uniform_int_distribution<int> uidX(m_pTerrain->GetWidth() - 500, m_pTerrain->GetWidth() - 1);
		std::uniform_int_distribution<int> uidZ(m_pTerrain->GetLength() - 500, m_pTerrain->GetLength() - 1);

		float randX = (float)uidX(dre);
		float randZ = (float)uidZ(dre);
		float randY = m_pTerrain->GetHeight(randX, randZ) + 50.0f;
		(*index)->Respawn(XMFLOAT3{ randX, randY, randZ });
	}
}

void CScene::BuildRandomEnemy(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	uniform_int_distribution<int> uidEnemy(0, Models.size() - 1);
	
	int Seed = uidEnemy(dre);
	CGameObject* pModel = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, Models[Seed]);
	switch (Seed)
	{
	case 0:
	{
		CGunshipObject* pGunshipObject = NULL;
		pGunshipObject = new CGunshipObject(m_pTerrain);
		pGunshipObject->SetChild(pModel, true);
		pGunshipObject->OnInitialize();
		pGunshipObject->isEnable = false;
		v_GameObjects.push_back(pGunshipObject);
	}
	break;
	case 1:
	{
		CSuperCobraObject* pSuperCobraObject = NULL;
		pSuperCobraObject = new CSuperCobraObject(m_pTerrain);
		pSuperCobraObject->SetChild(pModel, true);
		pSuperCobraObject->OnInitialize();
		pSuperCobraObject->isEnable = false;
		v_GameObjects.push_back(pSuperCobraObject);
	}
	break;
	case 2:
	{
		CMi24Object* pMi24Object = NULL;
		pMi24Object = new CMi24Object(m_pTerrain);
		pMi24Object->SetChild(pModel, true);
		pMi24Object->OnInitialize();
		pMi24Object->isEnable = false;
		v_GameObjects.push_back(pMi24Object);
	}
	break;
	case 3:
	{
		CApacheObject* pApacheObject = NULL;
		pApacheObject = new CApacheObject(m_pTerrain);
		pApacheObject->SetChild(pModel, true);
		pApacheObject->OnInitialize();
		pApacheObject->isEnable = false;
		v_GameObjects.push_back(pApacheObject);
	}
	break;
	}
}

void CScene::BuildDefaultEnemy(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (int i = 0; i < MAX_ENEMY; ++i)
		BuildRandomEnemy(pd3dDevice, pd3dCommandList);
}

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	XMFLOAT3 xmf3Scale(8.0f, 2.0f, 8.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.2f, 0.2f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("HeightMap.raw"), 512, 512, 512, 512, xmf3Scale, xmf4Color);
	BuildDefaultLightsAndMaterials();
	BuildDefaultEnemy(pd3dDevice, pd3dCommandList);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	for (int i = 0; i < v_GameObjects.size(); ++i)
		v_GameObjects[i]->Release();

	ReleaseShaderVariables();

	if (m_pLights) delete[] m_pLights;
	if (m_pTerrain) delete m_pTerrain;
}

ID3D12RootSignature *CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_ROOT_PARAMETER pd3dRootParameters[3];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 32;
	pd3dRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 4; //Lights
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256?? ????
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void **)&m_pcbMappedLights);
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));
}

void CScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}
}

void CScene::ReleaseUploadBuffers()
{
	for (int i = 0; i < v_GameObjects.size(); ++i)
		v_GameObjects[i]->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'W': v_GameObjects[0]->MoveForward(+1.0f); break;
		case 'S': v_GameObjects[0]->MoveForward(-1.0f); break;
		case 'A': v_GameObjects[0]->MoveStrafe(-1.0f); break;
		case 'D': v_GameObjects[0]->MoveStrafe(+1.0f); break;
		case 'Q': v_GameObjects[0]->MoveUp(+1.0f); break;
		case 'R': v_GameObjects[0]->MoveUp(-1.0f); break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return(false);
}

bool CScene::ProcessInput(UCHAR *pKeysBuffer)
{
	return(false);
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;
	RespawnTime += fTimeElapsed;
	if (RespawnTime > 3.0f)
	{
		RespawnTime = 0.0f;
		SpawnEnemy();
	}
	if (m_pLights)
	{
		m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights[1].m_xmf3Direction = m_pPlayer->GetLookVector();
	}
}

void CScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights

	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < v_GameObjects.size(); ++i)
	{
		if (v_GameObjects[i]->isEnable)
		{
			v_GameObjects[i]->Animate(m_fElapsedTime, NULL);
			v_GameObjects[i]->UpdateTransform(NULL);
			v_GameObjects[i]->Render(pd3dCommandList, pCamera);
		}
	}
}

void CScene::CheckCollide(CGameObject* target)
{
	for (int i = 0; i < v_GameObjects.size(); ++i)
	{
		if (v_GameObjects[i]->isEnable && v_GameObjects[i]->checkObjectCollision(target))
		{
			v_GameObjects[i]->isEnable = false;
			((CBullet*)target)->Reset();
		}
	}
}

void CScene::Update(CGameObject* m_pPlayer, float fTimeElapsed)
{
	for (int i = 0; i < v_GameObjects.size(); ++i)
	{
		if (v_GameObjects[i]->isEnable)
		{
			((CHellicopterObject*)v_GameObjects[i])->Update(m_pPlayer, fTimeElapsed);
		}
	}
}

void  CScene::CommandF(CGameObject* m_pPlayer)
{
	for (int i = 0; i < v_GameObjects.size(); ++i)
	{
		if (v_GameObjects[i]->isEnable)
		{
			((CHellicopterObject*)v_GameObjects[i])->CommandF4(m_pPlayer);
		}
	}
}
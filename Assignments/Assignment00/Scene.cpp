#include <random>
#include <cmath>
#include "stdafx.h"
#include "Scene.h"
#include "GraphicsPipeline.h"

std::random_device rdScene;
std::default_random_engine dreScene{ rdScene() };

CScene::CScene(CPlayer* pPlayer)
{
	m_pPlayer = pPlayer;
}

CScene::~CScene()
{
}

void CScene::BuildObjects()
{
	CExplosiveObject::PrepareExplosion();

	m_pRailObject = new CRail();
	m_pRailObject->randRail();
	m_pRailObject->setRail();

	CCartMesh* pCartMesh = new CCartMesh(4.0f, 4.0f, 6.0f);
	m_pCartObject = new CCartObject();
	m_pCartObject->SetMesh(pCartMesh);
	m_pCartObject->SetColor(RGB(0, 0, 0));
	m_pCartObject->m_pRail = &m_pRailObject->rails;
	m_pPlayer->m_pCart = m_pCartObject;


	float fHalfWidth = 0.0f, fHalfHeight = 0.0f, fHalfDepth = 0.0f;

	for (const XMFLOAT3& xmf : m_pRailObject->vkeyrails)
	{
		if (abs(xmf.x) > fHalfWidth)
			fHalfWidth = abs(xmf.x);

		if (abs(xmf.y) > fHalfHeight)
			fHalfHeight = abs(xmf.y);

		if (abs(xmf.z) > fHalfDepth)
			fHalfDepth = abs(xmf.z);
	}
	fHalfWidth += 20.0f;
	fHalfHeight += 20.0f;
	fHalfDepth += 20.0f;

	CWallMesh* pWallCubeMesh = new CWallMesh(fHalfWidth * 2.0f, fHalfHeight * 2.0f, fHalfDepth * 2.0f, 4);

	m_pWallsObject = new CWallsObject();
	m_pWallsObject->SetPosition(0.0f, 0.0f, 0.0f);
	m_pWallsObject->SetMesh(pWallCubeMesh);
	m_pWallsObject->SetColor(RGB(0, 0, 0));
	m_pWallsObject->m_pxmf4WallPlanes[0] = XMFLOAT4(+1.0f, 0.0f, 0.0f, fHalfWidth);
	m_pWallsObject->m_pxmf4WallPlanes[1] = XMFLOAT4(-1.0f, 0.0f, 0.0f, fHalfWidth);
	m_pWallsObject->m_pxmf4WallPlanes[2] = XMFLOAT4(0.0f, +1.0f, 0.0f, fHalfHeight);
	m_pWallsObject->m_pxmf4WallPlanes[3] = XMFLOAT4(0.0f, -1.0f, 0.0f, fHalfHeight);
	m_pWallsObject->m_pxmf4WallPlanes[4] = XMFLOAT4(0.0f, 0.0f, +1.0f, fHalfDepth);
	m_pWallsObject->m_pxmf4WallPlanes[5] = XMFLOAT4(0.0f, 0.0f, -1.0f, fHalfDepth);

	m_nObjects = 40;
	m_ppObjects = new CGameObject* [m_nObjects];
	GenObjects(fHalfWidth, fHalfHeight, fHalfDepth);

#ifdef _WITH_DRAW_AXIS
	m_pWorldAxis = new CGameObject();
	CAxisMesh* pAxisMesh = new CAxisMesh(0.5f, 0.5f, 0.5f);
	m_pWorldAxis->SetMesh(pAxisMesh);
#endif
}

void CScene::ReleaseObjects()
{
	if (CExplosiveObject::m_pExplosionMesh) CExplosiveObject::m_pExplosionMesh->Release();

	for (int i = 0; i < m_nObjects; i++) if (m_ppObjects[i]) delete m_ppObjects[i];
	if (m_ppObjects) delete[] m_ppObjects;

	if (m_pRailObject) delete m_pRailObject;

	if (m_pWallsObject) delete m_pWallsObject;

	if (m_pCartObject) delete m_pCartObject;

#ifdef _WITH_DRAW_AXIS
	if (m_pWorldAxis) delete m_pWorldAxis;
#endif
}

void CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}

void CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			CExplosiveObject* pExplosiveObject = (CExplosiveObject*)m_ppObjects[int(wParam - '1')];
			pExplosiveObject->m_bBlowingUp = true;
			break;
		}
		case 'A':
			for (int i = 0; i < m_nObjects; i++)
			{
				CExplosiveObject* pExplosiveObject = (CExplosiveObject*)m_ppObjects[i];
				pExplosiveObject->m_bBlowingUp = true;
			}
			break;
		case 'R':
			Reset();
			m_pPlayer->Reset();
			break;
		case VK_UP:
			m_pCartObject->SpeedUp();
			break;
		case VK_DOWN:
			m_pCartObject->SpeedDown();
		default:
			break;
		}
		break;
	default:
		break;
	}
}

CGameObject* CScene::PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera)
{
	XMFLOAT3 xmf3PickPosition;
	xmf3PickPosition.x = (((2.0f * xClient) / (float)pCamera->m_Viewport.m_nWidth) - 1) / pCamera->m_xmf4x4PerspectiveProject._11;
	xmf3PickPosition.y = -(((2.0f * yClient) / (float)pCamera->m_Viewport.m_nHeight) - 1) / pCamera->m_xmf4x4PerspectiveProject._22;
	xmf3PickPosition.z = 1.0f;

	XMVECTOR xmvPickPosition = XMLoadFloat3(&xmf3PickPosition);
	XMMATRIX xmmtxView = XMLoadFloat4x4(&pCamera->m_xmf4x4View);

	int nIntersected = 0;
	float fNearestHitDistance = FLT_MAX;
	CGameObject* pNearestObject = NULL;
	for (int i = 0; i < m_nObjects; i++)
	{
		float fHitDistance = FLT_MAX;
		nIntersected = m_ppObjects[i]->PickObjectByRayIntersection(xmvPickPosition, xmmtxView, &fHitDistance);
		if ((nIntersected > 0) && (fHitDistance < fNearestHitDistance))
		{
			fNearestHitDistance = fHitDistance;
			pNearestObject = m_ppObjects[i];
		}
	}
	return(pNearestObject);
}

void CScene::CheckObjectByObjectCollisions()
{
	for (int i = 0; i < m_nObjects; i++) m_ppObjects[i]->m_pObjectCollided = NULL;
	for (int i = 0; i < m_nObjects; i++)
	{
		for (int j = (i + 1); j < m_nObjects; j++)
		{
			if (m_ppObjects[i]->m_xmOOBB.Intersects(m_ppObjects[j]->m_xmOOBB))
			{
				m_ppObjects[i]->m_pObjectCollided = m_ppObjects[j];
				m_ppObjects[j]->m_pObjectCollided = m_ppObjects[i];
			}
		}
	}
	for (int i = 0; i < m_nObjects; i++)
	{
		if (m_ppObjects[i]->m_pObjectCollided)
		{
			XMFLOAT3 xmf3MovingDirection = m_ppObjects[i]->m_xmf3MovingDirection;
			float fMovingSpeed = m_ppObjects[i]->m_fMovingSpeed;
			m_ppObjects[i]->m_xmf3MovingDirection = m_ppObjects[i]->m_pObjectCollided->m_xmf3MovingDirection;
			m_ppObjects[i]->m_fMovingSpeed = m_ppObjects[i]->m_pObjectCollided->m_fMovingSpeed;
			m_ppObjects[i]->m_pObjectCollided->m_xmf3MovingDirection = xmf3MovingDirection;
			m_ppObjects[i]->m_pObjectCollided->m_fMovingSpeed = fMovingSpeed;
			m_ppObjects[i]->m_pObjectCollided->m_pObjectCollided = NULL;
			m_ppObjects[i]->m_pObjectCollided = NULL;
		}
	}
}

void CScene::CheckObjectByWallCollisions()
{
	for (int i = 0; i < m_nObjects; i++)
	{
		ContainmentType containType = m_pWallsObject->m_xmOOBB.Contains(m_ppObjects[i]->m_xmOOBB);
		switch (containType)
		{
		case DISJOINT:
		{
			int nPlaneIndex = -1;
			for (int j = 0; j < 6; j++)
			{
				PlaneIntersectionType intersectType = m_ppObjects[i]->m_xmOOBB.Intersects(XMLoadFloat4(&m_pWallsObject->m_pxmf4WallPlanes[j]));
				if (intersectType == BACK)
				{
					nPlaneIndex = j;
					break;
				}
			}
			if (nPlaneIndex != -1)
			{
				XMVECTOR xmvNormal = XMVectorSet(m_pWallsObject->m_pxmf4WallPlanes[nPlaneIndex].x, m_pWallsObject->m_pxmf4WallPlanes[nPlaneIndex].y, m_pWallsObject->m_pxmf4WallPlanes[nPlaneIndex].z, 0.0f);
				XMVECTOR xmvReflect = XMVector3Reflect(XMLoadFloat3(&m_ppObjects[i]->m_xmf3MovingDirection), xmvNormal);
				XMStoreFloat3(&m_ppObjects[i]->m_xmf3MovingDirection, xmvReflect);
			}
			break;
		}
		case INTERSECTS:
		{
			int nPlaneIndex = -1;
			for (int j = 0; j < 6; j++)
			{
				PlaneIntersectionType intersectType = m_ppObjects[i]->m_xmOOBB.Intersects(XMLoadFloat4(&m_pWallsObject->m_pxmf4WallPlanes[j]));
				if (intersectType == INTERSECTING)
				{
					nPlaneIndex = j;
					break;
				}
			}
			if (nPlaneIndex != -1)
			{
				XMVECTOR xmvNormal = XMVectorSet(m_pWallsObject->m_pxmf4WallPlanes[nPlaneIndex].x, m_pWallsObject->m_pxmf4WallPlanes[nPlaneIndex].y, m_pWallsObject->m_pxmf4WallPlanes[nPlaneIndex].z, 0.0f);
				XMVECTOR xmvReflect = XMVector3Reflect(XMLoadFloat3(&m_ppObjects[i]->m_xmf3MovingDirection), xmvNormal);
				XMStoreFloat3(&m_ppObjects[i]->m_xmf3MovingDirection, xmvReflect);
			}
			break;
		}
		case CONTAINS:
			break;
		}
	}
}

void CScene::CheckPlayerByWallCollision()
{
	BoundingOrientedBox xmOOBBPlayerMoveCheck;
	m_pWallsObject->m_xmOOBBPlayerMoveCheck.Transform(xmOOBBPlayerMoveCheck, XMLoadFloat4x4(&m_pWallsObject->m_xmf4x4World));
	XMStoreFloat4(&xmOOBBPlayerMoveCheck.Orientation, XMQuaternionNormalize(XMLoadFloat4(&xmOOBBPlayerMoveCheck.Orientation)));

	if (!xmOOBBPlayerMoveCheck.Intersects(m_pPlayer->m_xmOOBB)) m_pWallsObject->SetPosition(m_pPlayer->m_xmf3Position);
}

void CScene::CheckObjectByBulletCollisions()
{
	CBulletObject** ppBullets = ((CBarrelPlayer*)m_pPlayer)->m_ppBullets;
	for (int i = 0; i < m_nObjects; i++)
	{
		for (int j = 0; j < BULLETS; j++)
		{
			if (ppBullets[j]->m_bActive && m_ppObjects[i]->m_xmOOBB.Intersects(ppBullets[j]->m_xmOOBB))
			{
				CExplosiveObject* pExplosiveObject = (CExplosiveObject*)m_ppObjects[i];
				pExplosiveObject->m_bBlowingUp = true;
				ppBullets[j]->Reset();
			}
		}
	}
}

void CScene::Animate(float fElapsedTime)
{

	if (m_pCartObject) m_pCartObject->move(fElapsedTime);

	for (int i = 0; i < m_nObjects; i++) m_ppObjects[i]->Animate(fElapsedTime);

	CheckObjectByWallCollisions();

	CheckObjectByObjectCollisions();

	CheckObjectByBulletCollisions();
}

void CScene::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CGraphicsPipeline::SetViewport(&pCamera->m_Viewport);

	CGraphicsPipeline::SetViewPerspectiveProjectTransform(&pCamera->m_xmf4x4ViewPerspectiveProject);
	for (int i = 0; i < m_nObjects; i++) m_ppObjects[i]->Render(hDCFrameBuffer, pCamera);
	m_pWallsObject->Render(hDCFrameBuffer, pCamera);

	if (m_pPlayer) m_pPlayer->Render(hDCFrameBuffer, pCamera);
	if (m_pRailObject) m_pRailObject->Render(hDCFrameBuffer, pCamera);
	if (m_pCartObject) m_pCartObject->Render(hDCFrameBuffer, pCamera);

//UI
#ifdef _WITH_DRAW_AXIS
	CGraphicsPipeline::SetViewOrthographicProjectTransform(&pCamera->m_xmf4x4ViewOrthographicProject);
	m_pWorldAxis->SetRotationTransform(&m_pPlayer->m_xmf4x4World);
	m_pWorldAxis->Render(hDCFrameBuffer, pCamera);
#endif
}

void CScene::GenObjects(float width, float height, float depth)
{
	for (int i = 0; i < m_nObjects; ++i)
	{
		CCubeMesh* pCubeMesh = new CCubeMesh(20.0f, 20.0f, 20.0f);

		XMFLOAT3 pPosition = m_pPlayer->GetPosition();
		XMFLOAT4X4 mtxRotate = Matrix4x4::Identity();
		std::uniform_int_distribution<int> uidDepth(-width, +width);
		std::uniform_int_distribution<int> uidHeight(-height, +height);
		std::uniform_int_distribution<int> uidWidth(-depth, +depth);
		std::uniform_int_distribution<int> uidColor(0, 255);
		std::uniform_int_distribution<int> uidSpeed(20, 40);
		std::uniform_int_distribution<int> uidRSpeed(25, 40);
		std::uniform_int_distribution<int> uidDir(-1, 1);

		mtxRotate._11 = m_pPlayer->m_xmf3Right.x; mtxRotate._21 = m_pPlayer->m_xmf3Up.x; mtxRotate._31 = m_pPlayer->m_xmf3Look.x;
		mtxRotate._12 = m_pPlayer->m_xmf3Right.y; mtxRotate._22 = m_pPlayer->m_xmf3Up.y; mtxRotate._32 = m_pPlayer->m_xmf3Look.y;
		mtxRotate._13 = m_pPlayer->m_xmf3Right.z; mtxRotate._23 = m_pPlayer->m_xmf3Up.z; mtxRotate._33 = m_pPlayer->m_xmf3Look.z;

		XMFLOAT3 xmf3objPosition{ (float)uidWidth(dreScene), (float)uidHeight(dreScene), (float)uidDepth(dreScene) };
		XMFLOAT3 xmf3Direction{(float)uidDir(dreScene), (float)uidDir(dreScene) , (float)uidDir(dreScene) };
		while (xmf3Direction.x + xmf3Direction.y + xmf3Direction.z == 0)
		{
			xmf3Direction = XMFLOAT3{ (float)uidDir(dreScene), (float)uidDir(dreScene) , (float)uidDir(dreScene) };
		} 

		m_ppObjects[i] = new CExplosiveObject();
		m_ppObjects[i]->SetMesh(pCubeMesh);
		m_ppObjects[i]->SetColor(RGB(uidColor(dreScene), uidColor(dreScene), uidColor(dreScene)));
		m_ppObjects[i]->SetPosition(xmf3objPosition);
		m_ppObjects[i]->SetRotationAxis(XMFLOAT3(1.0f, 1.0f, 0.0f));
		m_ppObjects[i]->SetRotationSpeed((float)uidRSpeed(dreScene));
		m_ppObjects[i]->SetMovingDirection(xmf3Direction);
		m_ppObjects[i]->SetMovingSpeed((float)uidSpeed(dreScene));
	}
}

void CScene::Reset()
{
	ReleaseObjects();
	BuildObjects();
}
#pragma once
#include <array>
#include <vector>
#include "GameObject.h"

#define KEY_RAIL_NUM 10

class CRail
{
public:
	std::vector<XMFLOAT3> vkeyrails;
	std::vector<CRailObject*> rails;
	
public:
	CRail();
	~CRail();

	void setRail();
	void randRail();
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);
};


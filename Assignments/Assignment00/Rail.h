#pragma once
#include <array>
#include <vector>
#include "GameObject.h"

#define KEY_RAIL_NUM 6

class CRail
{
public:
	std::array<XMFLOAT3, KEY_RAIL_NUM> vkeyrails;
	std::vector<CRailObject*> rails;
	
public:
	CRail();
	~CRail();

	void setRail();
	void randRail();
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);
};


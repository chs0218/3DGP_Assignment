#pragma once
#include <array>
#include <vector>
#include "GameObject.h"

#define KEY_RAIL_NUM 10

class CRail
{
private:
	std::vector<std::array<float, 3>> vkeyrails;
	std::vector<CRailObject*> rails;
	
public:
	CRail();
	~CRail();

	void setRail();
	void randRail();
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);
};


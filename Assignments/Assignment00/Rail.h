#pragma once
#include <array>
#include <vector>
#include "GameObject.h"

#define KEY_RAIL_NUM 5

class CRail
{
private:
	std::vector<std::array<float, 6>> vkeyrails;
	std::vector<CRailObject*> rails;
	
public:
	CRail();
	~CRail();

	void setRail();
	void randRail();
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);
};


#include <random>
#include "Rail.h"

std::random_device rd;
std::default_random_engine dre{ rd() };

CRail::CRail()
{
}

CRail::~CRail()
{
	
}

void CRail::setRail()
{
	CCubeMesh* tmpRailMesh = new CCubeMesh(4.0, 2.0, 4.0);
	for (int i = 0; i < (vkeyrails.size() - 1); ++i)
	{
		for (int j = 0; j < 20; ++j)
		{
			rails.push_back(new CRailObject(vkeyrails[i], vkeyrails[i + 1], tmpRailMesh, (float)j));
		}
		
	}
}


void CRail::randRail()
{
	std::uniform_int_distribution<int> uid(20, 50);

	for (int i = 0; i < KEY_RAIL_NUM; ++i)
	{
		std::array<float, 6> tmp;
		
		if (i == 0)
		{
			tmp.fill(0.0f);
			tmp[1] = (-1.0f);
		}

		else
		{
			for (int j = 0; j < 3; ++j)
			{
				tmp[j] = vkeyrails[i - 1][j] + (float)(uid(dre));

			}
		}

		vkeyrails.push_back(tmp);
	}

}

void CRail::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	for (auto i : rails)
		i->Render(hDCFrameBuffer, pCamera);
}
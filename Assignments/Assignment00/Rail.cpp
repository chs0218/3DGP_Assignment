#include <random>
#include "Rail.h"

// std::random_device rd;
std::default_random_engine dre{};

CRail::CRail()
{
}

CRail::~CRail()
{
	
}

void CRail::setRail()
{
	CCubeMesh* tmpRailMesh = new CCubeMesh(4.0, 2.0, 4.0);
	for (int i = 0; i < vkeyrails.size(); ++i)
	{
		std::array<std::array<float, 3>, 4> tmp;
		int cur_index = vkeyrails.size() - 1 + i;

		for (int l = 0; l < 4; ++l)
		{
			if (cur_index < vkeyrails.size())
				tmp[l] = vkeyrails[cur_index];

			else
			{
				cur_index = cur_index % vkeyrails.size();
				tmp[l] = vkeyrails[cur_index];
			}

			cur_index += 1;
		}

		float dis = sqrt(pow(tmp[1][0] - tmp[2][0], 2) + pow(tmp[1][1] - tmp[2][1], 2) + pow(tmp[1][2] - tmp[2][2], 2));
		float para = dis / 4.0f;

		for (int j = 0; j < (para - 1); ++j)
		{
			if (rails.size() == 0)
			{
				std::array<float, 3> tmpradius{ 0.0, 0.0, 1.0 };
				rails.push_back(new CRailObject(tmp, tmpRailMesh, (float)j * 100.0 / para));
				rails[0]->vradius = tmpradius;
				rails[0]->rotateToVec(rails[0]->getDirection((tmp), (float)j * 100.0 / para));
			}
			else
			{
				std::array<float, 3> tmpradius = rails.back()->vradius;
				rails.push_back(new CRailObject(tmp, tmpRailMesh, (float)j * 100.0 / para));
				rails.back()->vradius = tmpradius;
				rails.back()->rotateToVec(rails[0]->getDirection((tmp), (float)j * 100.0 / para));
			}
		}

	}

	/*for (int i = 0; i < rails.size(); ++i)
	{
		XMFLOAT3 now, next;
		if (i == rails.size() - 1)
		{
			now = rails[i]->GetPosition();
			next = rails[0]->GetPosition();
			rails[i]->rotateToVec((next.x - now.x), (next.y - now.y), (next.z - now.z));
		}
		else
		{
			now = rails[i]->GetPosition();
			next = rails[i + 1]->GetPosition();
			rails[i]->rotateToVec((next.x - now.x), (next.y - now.y), (next.z - now.z));
		}
	}*/
}


void CRail::randRail()
{
	std::uniform_int_distribution<int> uid(-100, 100);

	for (int i = 0; i < KEY_RAIL_NUM; ++i)
	{
		std::array<float, 3> tmp;
		
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

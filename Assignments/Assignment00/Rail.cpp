#include <random>
#include "Rail.h"

std::random_device rdRail;
std::default_random_engine dreRail{ rdRail() };

CRail::CRail()
{
}

CRail::~CRail()
{
	
}

void CRail::setRail()
{
	CCubeMesh* tmpRailMesh = new CCubeMesh(4.0, 2.0, 4.0);
	XMFLOAT3 look = { 0.0, 0.0, 1.0 }, next;
	XMVECTOR pre, now;

	for (int i = 0; i < vkeyrails.size(); ++i)
	{
		std::array<XMFLOAT3, 4> tmp;
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

		float dis = sqrt(powf((tmp[1].x - tmp[2].x), 2) + powf((tmp[1].y - tmp[2].y), 2) + powf((tmp[1].z - tmp[2].z), 2));
		dis = dis / 4.0f;

		for (int j = 0; j < dis; ++j)
		{
			std::array<XMVECTOR, 4> vPoints{ XMLoadFloat3(&tmp[0]),
		XMLoadFloat3(&tmp[1]),  XMLoadFloat3(&tmp[2]),  XMLoadFloat3(&tmp[3]) };
			rails.push_back(new CRailObject(vPoints, tmpRailMesh, (float)j * 100.0 / dis));
		} 

	}

	for (int i = 0; i < rails.size() - 1; ++i)
	{
		next = { rails[i + 1]->GetPosition().x - rails[i]->GetPosition().x,
					rails[i + 1]->GetPosition().y - rails[i]->GetPosition().y,
					rails[i + 1]->GetPosition().z - rails[i]->GetPosition().z };


		pre = XMLoadFloat3(&look);
		now = XMLoadFloat3(&next);

		rails[i]->rotateToVec(pre, now);
	}

	next = {rails.front()->GetPosition().x - rails.back()->GetPosition().x,
					rails.front()->GetPosition().y - rails.back()->GetPosition().y,
					rails.front()->GetPosition().z - rails.back()->GetPosition().z };
	

	pre = XMLoadFloat3(&look);
	now = XMLoadFloat3(&next);

	rails.back()->rotateToVec(pre, now);
}


void CRail::randRail()
{
	std::uniform_int_distribution<int> uid(-100, 100);
	std::uniform_int_distribution<int> uidmp(0, 1);
	for (int i = 0; i < KEY_RAIL_NUM; ++i)
	{
		XMFLOAT3 tmp;
		
		if (i == 0)
		{
			tmp.x = 0.0f;
			tmp.y = -1.0f;
			tmp.z = 0.0f;
		}

		else
		{
			tmp.x = vkeyrails[i - 1].x + (float)(uid(dreRail));
			tmp.y = vkeyrails[i - 1].y + (float)(uid(dreRail));
			tmp.z = vkeyrails[i - 1].z + (float)(uid(dreRail));
		}
		vkeyrails.push_back(tmp);
	}

}

void CRail::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	for (auto i : rails)
		i->Render(hDCFrameBuffer, pCamera);
}
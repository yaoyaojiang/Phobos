#include "AreaClass.h"

#include <Ext/Scenario/Body.h>
#include <Utilities/SavegameDef.h>
#include <algorithm>
#include <functional> 

std::vector<AreaClass*> AreaClass::Array;


AreaClass::AreaClass(int id, std::vector<CellStruct> cells)
	: ID(id), Cells(std::move(cells)) { }
AreaClass::AreaClass()
	: ID(0), Cells()  // 显式初始化成员
{
}
template <typename T>
bool AreaClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->ID)
		.Process(this->Cells)
		.Success();
}

bool AreaClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return Serialize(stm);
}

bool AreaClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<AreaClass*>(this)->Serialize(stm);
}


void AreaClass::FillQuadrilateral(int wp1, int wp2, int wp3, int wp4)
{
	// 获取四个路径点坐标
	CellStruct p1 = ScenarioClass::Instance()->GetWaypointCoords(wp1);
	CellStruct p2 = ScenarioClass::Instance()->GetWaypointCoords(wp2);
	CellStruct p3 = ScenarioClass::Instance()->GetWaypointCoords(wp3);
	CellStruct p4 = ScenarioClass::Instance()->GetWaypointCoords(wp4);

	// 清空现有数据
	Cells.clear();

	// 添加四个顶点
	Cells.insert(Cells.end(), { p1, p2, p3, p4 });

	// 添加四条边上的所有格子
	AddEdgeCells(p1, p2, Cells);
	AddEdgeCells(p2, p3, Cells);
	AddEdgeCells(p3, p4, Cells);
	AddEdgeCells(p4, p1, Cells);

	// 计算包围盒
	short minX = std::min({ p1.X, p2.X, p3.X, p4.X });
	short maxX = std::max({ p1.X, p2.X, p3.X, p4.X });
	short minY = std::min({ p1.Y, p2.Y, p3.Y, p4.Y });
	short maxY = std::max({ p1.Y, p2.Y, p3.Y, p4.Y });

	// 遍历内部格子
	for (short x = minX; x <= maxX; ++x)
	{
		for (short y = minY; y <= maxY; ++y)
		{
			CellStruct cell { x, y };

			// 跳过已添加的边和顶点
			if (std::find(Cells.begin(), Cells.end(), cell) != Cells.end())
			{
				continue;
			}

			// 判断内部格子
			if (IsCellInQuadrilateral(cell, p1, p2, p3, p4))
			{
				Cells.push_back(cell);
			}
		}
	}

	// 去重处理
	std::sort(Cells.begin(), Cells.end(),
		[](const CellStruct& a, const CellStruct& b)
 {
	 // 先按X排序，再按Y排序
	 return (a.X < b.X) || (a.X == b.X && a.Y < b.Y);
		}
	);

	// 修正2：自定义去重规则
	auto last = std::unique(Cells.begin(), Cells.end(),[](const CellStruct& a, const CellStruct& b) {
	 return a.X == b.X && a.Y == b.Y;
		}
	);

	Cells.erase(last, Cells.end());
}

// Bresenham算法实现
void AreaClass::AddEdgeCells(
	const CellStruct& start,
	const CellStruct& end,
	std::vector<CellStruct>& cells)
{
	CellStruct current = start;
	int dx = abs(end.X - current.X);
	int dy = -abs(end.Y - current.Y);
	int sx = current.X < end.X ? 1 : -1;
	int sy = current.Y < end.Y ? 1 : -1;
	int err = dx + dy;

	while (true)
	{
		// 添加当前格子（自动去重）
		if (std::find(cells.begin(), cells.end(), current) == cells.end())
		{
			cells.push_back(current);
		}

		if (current.X == end.X && current.Y == end.Y) break;

		int e2 = 2 * err;
		if (e2 >= dy)
		{
			if (current.X == end.X) break;
			err += dy;
			current.X += sx;
		}
		if (e2 <= dx)
		{
			if (current.Y == end.Y) break;
			err += dx;
			current.Y += sy;
		}
	}
}

// 改进的射线法判断
bool AreaClass::IsCellInQuadrilateral(
	const CellStruct& cell,
	const CellStruct& p1,
	const CellStruct& p2,
	const CellStruct& p3,
	const CellStruct& p4)
{
	const CellStruct polygon[] = { p1, p2, p3, p4, p1 };
	int intersectCount = 0;

	for (int i = 0; i < 4; ++i)
	{
		const CellStruct& a = polygon[i];
		const CellStruct& b = polygon[i + 1];

		// 处理水平边
		if (a.Y == b.Y)
		{
			if (cell.Y == a.Y && cell.X >= std::min(a.X, b.X) && cell.X <= std::max(a.X, b.X))
			{
				return true;
			}
			continue;
		}

		// 计算射线交点
		if ((cell.Y > a.Y && cell.Y <= b.Y) || (cell.Y > b.Y && cell.Y <= a.Y))
		{
			float t = static_cast<float>(cell.Y - a.Y) / (b.Y - a.Y);
			float xIntersect = a.X + t * (b.X - a.X);

			// 精确处理垂直线段
			if (a.X == b.X)
			{
				if (cell.X == a.X) return true;
				if (cell.X < xIntersect) intersectCount++;
			}
			else
			{
				if (cell.X < xIntersect) intersectCount++;
			}
		}
	}

	return (intersectCount % 2) == 1;
}

void AreaClass::Clear()
{
	for (auto* ptr : Array)
	{
		delete ptr;
	}
	Array.clear();
}

bool AreaClass::LoadGlobals(PhobosStreamReader& stm)
{
	return stm
		.Process(Array)
		.Success();
}

bool AreaClass::SaveGlobals(PhobosStreamWriter& stm)
{
	return stm
		.Process(Array)
		.Success();
}

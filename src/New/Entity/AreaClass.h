#pragma once

#include <GeneralStructures.h>

#include <vector>
#include <Utilities/Savegame.h>
class AreaClass
{

public:
	static std::vector<AreaClass*> Array;
	int ID;
    std::vector<CellStruct> Cells;

	AreaClass();  // 显式声明默认构造函数 <--- 添加此行
	AreaClass(int id, std::vector<CellStruct> cells);
	~AreaClass() = default;
	// 已有代码...
	void  FillQuadrilateral(int wp1, int wp2, int wp3, int wp4);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	static void Clear();
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

private:

	static void AddEdgeCells(
		const CellStruct& start,
		const CellStruct& end,
		std::vector<CellStruct>& cells
	);

	bool IsCellInQuadrilateral(const CellStruct& cell,
	const CellStruct& p1,
	const CellStruct& p2,
	const CellStruct& p3,
	const CellStruct& p4);
	template <typename T>
	bool Serialize(T& Stm);

};

#pragma once
#include <GeneralStructures.h>
#include <vector>
#include <unordered_set>
#include <Utilities/Savegame.h>

class HouseClass; // 或包含 HouseClass.h

class AreaClass
{
public:
	static std::vector<AreaClass*> Array;
	int ID;
	std::vector<CellStruct> Cells;
	HouseClass* CaptureHouse;
	int CaptureObjectNumber;

	AreaClass();
	AreaClass(int id, std::vector<CellStruct> cells, HouseClass* capturehouse,int CaptureObjectNumber);
	~AreaClass() = default;

	void FillQuadrilateral(int wp1, int wp2, int wp3, int wp4);
	bool IsInArea(CellStruct pCell);
	void BuildCellSet();
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	static void Clear();
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

private:
	static void AddEdgeCells(const CellStruct& start, const CellStruct& end, std::vector<CellStruct>& cells);
	bool IsCellInQuadrilateral(const CellStruct& cell, const CellStruct& p1, const CellStruct& p2, const CellStruct& p3, const CellStruct& p4);
	std::unordered_set<size_t> CellSet;
	template <typename T>
	bool Serialize(T& Stm);
};

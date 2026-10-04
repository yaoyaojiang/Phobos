#include "Body.h"

#include <algorithm>
#include <map>

#include <Helpers/Macro.h>

#include <Utilities/TemplateDef.h>

#include <MapClass.h>

CellExt::ExtContainer CellExt::ExtMap;

namespace
{
	// 每个单位当前登记的格，按单位索引，避免多单位共享状态的竞态
	std::map<FootClass*, CellStruct> AirUnitLastCell;
}

// =============================
// accessors
std::vector<TechnoClass*>& CellExt::GetAirUnits(CellClass* pCell)
{
	static std::vector<TechnoClass*> Empty;

	if (!pCell)
		return Empty;

	auto* pExt = ExtMap.Find(pCell);
	return pExt ? pExt->AirUnits : Empty;
}

void CellExt::AddAirUnit(CellClass* pCell, TechnoClass* pUnit)
{
	if (!pCell || !pUnit)
		return;

	auto* pExt = ExtMap.Find(pCell);
	if (!pExt)
	{
		// 单元可能先于该格 ExtData 分配被登记，直接返回会静默漏掉该单元
		pExt = ExtMap.TryAllocate(pCell);
		if (!pExt)
			return;
	}

	pExt->AddAirUnit(pUnit);
}

void CellExt::RemoveAirUnit(CellClass* pCell, TechnoClass* pUnit)
{
	if (!pCell || !pUnit)
		return;

	auto* pExt = ExtMap.Find(pCell);
	if (!pExt)
		return;

	pExt->RemoveAirUnit(pUnit);
}

void CellExt::ExtData::AddAirUnit(TechnoClass* pUnit)
{
	if (!pUnit)
		return;

	if (std::find(this->AirUnits.begin(), this->AirUnits.end(), pUnit) == this->AirUnits.end())
		this->AirUnits.push_back(pUnit);
}

void CellExt::ExtData::RemoveAirUnit(TechnoClass* pUnit)
{
	if (!pUnit)
		return;

	this->AirUnits.erase(std::remove(this->AirUnits.begin(), this->AirUnits.end(), pUnit), this->AirUnits.end());
}

void CellExt::UpdateAirUnit(FootClass* pLinkedTo)
{
	if (!pLinkedTo || !pLinkedTo->IsAlive)
		return;

	CellStruct cur = pLinkedTo->GetMapCoords();
	auto it = AirUnitLastCell.find(pLinkedTo);

	if (it == AirUnitLastCell.end())
	{
		// 首次登记：直接把单位加入当前格
		if (auto* c = MapClass::Instance->TryGetCellAt(cur))
			CellExt::AddAirUnit(c, pLinkedTo);
		AirUnitLastCell[pLinkedTo] = cur;
	}
	else if (it->second != cur)
	{
		// 换格：从旧格移除，加入新格
		if (auto* c = MapClass::Instance->TryGetCellAt(it->second))
			CellExt::RemoveAirUnit(c, pLinkedTo);
		if (auto* c = MapClass::Instance->TryGetCellAt(cur))
			CellExt::AddAirUnit(c, pLinkedTo);
		it->second = cur;
	}
}

void CellExt::ClearAirUnit(FootClass* pUnit)
{
	AirUnitLastCell.erase(pUnit);
}

std::vector<TechnoClass*> CellExt::GetAllTrackedAirUnits()
{
	std::vector<TechnoClass*> result;
	result.reserve(AirUnitLastCell.size());
	for (auto& pair : AirUnitLastCell)
		result.push_back(pair.first);
	return result;
}

void CellExt::PointerGotInvalid(void* ptr, bool removed)
{
	if (!ptr)
		return;

	// 只有被摧毁的是已登记的空气单位时才需要清理，其余对象直接忽略，避免高昂的全格遍历
	auto it = AirUnitLastCell.find(reinterpret_cast<FootClass*>(ptr));
	if (it == AirUnitLastCell.end())
		return;

	// 从该单位登记所在格的 AirUnits 中移除，再清除全局追踪表
	if (auto* pCell = MapClass::Instance->TryGetCellAt(it->second))
		CellExt::RemoveAirUnit(pCell, reinterpret_cast<TechnoClass*>(ptr));

	AirUnitLastCell.erase(it);
}

// =============================
// load / save
template <typename T>
void CellExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->AirUnits)
		;
}

void CellExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<CellClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void CellExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<CellClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

void CellExt::ExtData::InvalidatePointer(void* ptr, bool removed)
{
	if (ptr)
		CellExt::ClearAirUnit(reinterpret_cast<FootClass*>(ptr));

	if (!this->AirUnits.empty() && ptr != nullptr)
	{
		this->AirUnits.erase(
			std::remove(this->AirUnits.begin(), this->AirUnits.end(), reinterpret_cast<TechnoClass*>(ptr)),
			this->AirUnits.end());
	}
}



// =============================
// container

CellExt::ExtContainer::ExtContainer() : Container("CellClass") { }
CellExt::ExtContainer::~ExtContainer() = default;

bool CellExt::ExtContainer::InvalidateExtDataIgnorable(void* const ptr) const
{
	return true;
}

// =============================
// container hooks

DEFINE_HOOK(0x47BDA1, CellClass_CTOR, 0x5)
{
	GET(CellClass*, pItem, ESI);

	CellExt::ExtMap.Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x47BB60, CellClass_DTOR, 0x6)
{
	GET(CellClass*, pItem, ECX);

	CellExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x483C10, CellClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x4839F0, CellClass_SaveLoad_Prefix, 0x7)
{
	GET_STACK(CellClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	CellExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x483C00, CellClass_Load_Suffix, 5)
{
	CellExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x483C79, CellClass_Save_Suffix, 0x6)
{
	CellExt::ExtMap.SaveStatic();
	return 0;
}

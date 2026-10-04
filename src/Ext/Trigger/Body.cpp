#include "Body.h"

#include <Ext/House/Body.h>
#include <Misc/SyncLogging.h>

TriggerExt::ExtContainer TriggerExt::ExtMap;




HouseClass* TriggerExt::GetOwnerHouse(TriggerClass* pTrigger)
{
	if (!pTrigger)
		return nullptr;

	return pTrigger->House;
}


// =============================
// load / save

template <typename T>
void TriggerExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Cell)
		.Process(this->AttachedTechnos)
		;
}

void TriggerExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TriggerClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TriggerExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TriggerClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

void TriggerExt::ExtData::InitializeConstants()
{

}

// =============================
// container

TriggerExt::ExtContainer::ExtContainer() : Container("TriggerClass") { }
TriggerExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x72612E, TriggerClass_CTOR, 0x7)
{
	GET(TriggerClass*, pItem, ESI);
	TriggerExt::ExtMap.TryAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x726981, TriggerClass_DTOR, 0x9)
{
	GET(TriggerClass*, pItem, ESI);
	TriggerExt::ExtMap.Remove(pItem);
	return 0;
}
DEFINE_HOOK_AGAIN(0x726860, TriggerClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x7268D0, TriggerClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(TriggerClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TriggerExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x7268BE, TriggerClass_Load_Suffix, 0x6)
{
	TriggerExt::ExtMap.LoadStatic();
	return 0; 
}

DEFINE_HOOK(0x7268EA, TriggerClass_Save_Suffix, 0x5)
{
	TriggerExt::ExtMap.SaveStatic();
	return 0;
}

// Field D0 in TriggerClass is mostly unused so by removing the few uses it has it can be used to store TriggerExt pointer.

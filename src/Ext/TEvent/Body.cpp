#include "Body.h"

#include <Utilities/SavegameDef.h>
#include <New/Entity/ShieldClass.h>
#include <Ext/Scenario/Body.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <AircraftClass.h>
#include <HouseClass.h>
#include <SuperWeaponTypeClass.h>
#include <SuperClass.h>
#include <MessageListClass.h>
#include <TlHelp32.h>
#pragma comment(lib, "psapi.lib") // 链接psapi库  

//Static init
TEventExt::ExtContainer TEventExt::ExtMap;

// =============================
// load / save

template <typename T>
void TEventExt::ExtData::Serialize(T& Stm)
{
	//Stm;
}

void TEventExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TEventClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TEventExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TEventClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TEventExt::Execute(TEventClass* pThis, int iEvent, HouseClass* pHouse, ObjectClass* pObject,
	CDTimerClass* pTimer, bool* isPersitant, TechnoClass* pSource, bool& bHandled)
{
	bHandled = true;
	switch (static_cast<PhobosTriggerEvent>(pThis->EventKind))
	{
		// helper struct
		struct and_with { bool operator()(int a, int b) { return a & b; } };

	case PhobosTriggerEvent::LocalVariableGreaterThan:
		return TEventExt::VariableCheck<false, std::greater<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableLessThan:
		return TEventExt::VariableCheck<false, std::less<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableEqualsTo:
		return TEventExt::VariableCheck<false, std::equal_to<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableNotEqualsTo:
		return TEventExt::VariableCheck<false, std::not_equal_to<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsTo:
		return TEventExt::VariableCheck<false, std::greater_equal<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsTo:
		return TEventExt::VariableCheck<false, std::less_equal<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableAndIsTrue:
		return TEventExt::VariableCheck<false, and_with>(pThis);
	case PhobosTriggerEvent::GlobalVariableGreaterThan:
		return TEventExt::VariableCheck<true, std::greater<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableLessThan:
		return TEventExt::VariableCheck<true, std::less<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableEqualsTo:
		return TEventExt::VariableCheck<true, std::equal_to<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsTo:
		return TEventExt::VariableCheck<true, std::greater_equal<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsTo:
		return TEventExt::VariableCheck<true, std::less_equal<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableAndIsTrue:
		return TEventExt::VariableCheck<true, and_with>(pThis);

	case PhobosTriggerEvent::LocalVariableGreaterThanLocalVariable:
		return TEventExt::VariableCheckBinary<false, false, std::greater<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableLessThanLocalVariable:
		return TEventExt::VariableCheckBinary<false, false, std::less<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableEqualsToLocalVariable:
		return TEventExt::VariableCheckBinary<false, false, std::equal_to<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableNotEqualsToLocalVariable:
		return TEventExt::VariableCheckBinary<false, false, std::not_equal_to<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToLocalVariable:
		return TEventExt::VariableCheckBinary<false, false, std::greater_equal<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToLocalVariable:
		return TEventExt::VariableCheckBinary<false, false, std::less_equal<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableAndIsTrueLocalVariable:
		return TEventExt::VariableCheckBinary<false, false, and_with>(pThis);
	case PhobosTriggerEvent::GlobalVariableGreaterThanLocalVariable:
		return TEventExt::VariableCheckBinary<false, true, std::greater<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableLessThanLocalVariable:
		return TEventExt::VariableCheckBinary<false, true, std::less<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableEqualsToLocalVariable:
		return TEventExt::VariableCheckBinary<false, true, std::equal_to<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToLocalVariable:
		return TEventExt::VariableCheckBinary<false, true, std::greater_equal<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToLocalVariable:
		return TEventExt::VariableCheckBinary<false, true, std::less_equal<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableAndIsTrueLocalVariable:
		return TEventExt::VariableCheckBinary<false, true, and_with>(pThis);

	case PhobosTriggerEvent::LocalVariableGreaterThanGlobalVariable:
		return TEventExt::VariableCheckBinary<true, false, std::greater<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableLessThanGlobalVariable:
		return TEventExt::VariableCheckBinary<true, false, std::less<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableEqualsToGlobalVariable:
		return TEventExt::VariableCheckBinary<true, false, std::equal_to<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToGlobalVariable:
		return TEventExt::VariableCheckBinary<true, false, std::greater_equal<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToGlobalVariable:
		return TEventExt::VariableCheckBinary<true, false, std::less_equal<int>>(pThis);
	case PhobosTriggerEvent::LocalVariableAndIsTrueGlobalVariable:
		return TEventExt::VariableCheckBinary<true, false, and_with>(pThis);
	case PhobosTriggerEvent::GlobalVariableGreaterThanGlobalVariable:
		return TEventExt::VariableCheckBinary<true, true, std::greater<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableLessThanGlobalVariable:
		return TEventExt::VariableCheckBinary<true, true, std::less<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableEqualsToGlobalVariable:
		return TEventExt::VariableCheckBinary<true, true, std::equal_to<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToGlobalVariable:
		return TEventExt::VariableCheckBinary<true, true, std::greater_equal<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToGlobalVariable:
		return TEventExt::VariableCheckBinary<true, true, std::less_equal<int>>(pThis);
	case PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable:
		return TEventExt::VariableCheckBinary<true, true, and_with>(pThis);

	case PhobosTriggerEvent::ShieldBroken:
		return ShieldClass::ShieldIsBrokenTEvent(pObject);
	case PhobosTriggerEvent::HouseOwnsTechnoType:
		return TEventExt::HouseOwnsTechnoTypeTEvent(pThis);
	case PhobosTriggerEvent::HouseDoesntOwnTechnoType:
		return TEventExt::HouseDoesntOwnTechnoTypeTEvent(pThis);
	case PhobosTriggerEvent::SuperWeaponTimerUp:
		return TEventExt::SuperWeaponTimerUp(pThis, pHouse);
	case PhobosTriggerEvent::OwnerApproachWaypoint:
		return TEventExt::OwnerApproachWaypoint(pThis, pHouse);
	case PhobosTriggerEvent::ContainsShroud:
		return TEventExt::ContainsShroud(pThis, pObject);
	case PhobosTriggerEvent::Cheating:
		return TEventExt::Cheating(pThis);
	case PhobosTriggerEvent::OuterVariableEqual:
		return TEventExt::OuterVariableEqual(pThis);
	case PhobosTriggerEvent::OuterVariableBigger:
		return TEventExt::OuterVariableBigger(pThis);
	case PhobosTriggerEvent::OuterVariableSmaller:
		return TEventExt::OuterVariableSmaller(pThis);
	case PhobosTriggerEvent::OuterVariableNotEqual:
		return TEventExt::OuterVariableNotEqual(pThis);
	default:
		bHandled = false;
		return true;
	};
}

wchar_t* ConvertW(const char* asciiStr)
{
	size_t len = strlen(asciiStr) + 1; // 包括null终止符  
	wchar_t* wideStr = new wchar_t[len];
	for (size_t i = 0; i < len; ++i)
	{
		wideStr[i] = static_cast<wchar_t>(asciiStr[i]);
	}
	return wideStr;
}
template<bool IsGlobal, class _Pr>
bool TEventExt::VariableCheck(TEventClass* pThis)
{
	auto itr = ScenarioExt::Global()->Variables[IsGlobal].find(pThis->Value);

	if (itr != ScenarioExt::Global()->Variables[IsGlobal].end())
	{
		// We uses TechnoName for our operator number
		int nOpt = atoi(pThis->String);
		return _Pr()(itr->second.Value, nOpt);
	}

	return false;
}

template<bool IsSrcGlobal, bool IsGlobal, class _Pr>
bool TEventExt::VariableCheckBinary(TEventClass* pThis)
{
	auto itr = ScenarioExt::Global()->Variables[IsGlobal].find(pThis->Value);

	if (itr != ScenarioExt::Global()->Variables[IsGlobal].end())
	{
		// We uses TechnoName for our src variable index
		int nSrcVariable = atoi(pThis->String);
		auto itrsrc = ScenarioExt::Global()->Variables[IsSrcGlobal].find(nSrcVariable);

		if (itrsrc != ScenarioExt::Global()->Variables[IsSrcGlobal].end())
			return _Pr()(itr->second.Value, itrsrc->second.Value);
	}

	return false;
}

bool TEventExt::HouseOwnsTechnoTypeTEvent(TEventClass* pThis)
{
	auto pType = TechnoTypeClass::Find(pThis->String);
	if (!pType)
		return false;

	auto pHouse = HouseClass::FindByIndex(pThis->Value);
	if (!pHouse)
		return false;

	return pHouse->CountOwnedNow(pType) > 0;
}

bool TEventExt::HouseDoesntOwnTechnoTypeTEvent(TEventClass* pThis)
{
	return !TEventExt::HouseOwnsTechnoTypeTEvent(pThis);
}
void splitBySymbol(const char* str, char delim, char result[][32], int resultSize)
{
	int index = 0; // 结果数组的索引  
	const char* tokenStart = str; // 当前token的起始位置  
	const char* tokenEnd = str; // 当前token的结束位置  

	// 遍历字符串直到找到足够的token或到达字符串末尾  
	while (*tokenEnd && index < resultSize - 1)
	{ // 减1是为了给最后一个token的null终止符留空间  
		if (*tokenEnd == delim)
		{
			// 复制当前token到结果数组  
			strncpy(result[index], tokenStart, tokenEnd - tokenStart);
			result[index][tokenEnd - tokenStart] = '\0'; // 添加null终止符  
			index++;
			tokenStart = tokenEnd + 1; // 移动到下一个token的起始位置  
		}
		tokenEnd++; // 移动到下一个字符  
	}

	// 复制最后一个token（如果有的话）  
	if (*tokenStart)
	{
		strncpy(result[index], tokenStart, strlen(tokenStart) + 1); // 包括null终止符  
	}
}
bool TEventExt::OuterVariableEqual(TEventClass* pThis)
{
	const auto spcialText = pThis->String;
	const int maxTokens = 3;
	char result[maxTokens][32];
	char delimiter = '@';
	splitBySymbol(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto KeyName = result[1];
	const auto VariableName = result[2];
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);
	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
	{
		delete pINI;
		return false;
	}
	int target = pThis->Value;
	int value = pINI->ReadInteger(KeyName, VariableName, 0);
	delete pINI;
	if (value ==target)
		return true;
	else return false;
}
bool TEventExt::OuterVariableBigger(TEventClass* pThis)
{
	const auto spcialText = pThis->String;
	const int maxTokens = 3;
	char result[maxTokens][32];
	char delimiter = '@';
	splitBySymbol(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto KeyName = result[1];
	const auto VariableName = result[2];
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);
	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
	{
		delete pINI;
		return false;
	}
	int target = pThis->Value;
	int value = pINI->ReadInteger(KeyName, VariableName, 0);
	delete pINI;
	if (value>target)
	return true;
	else return false;
}
bool TEventExt::OuterVariableSmaller(TEventClass* pThis)
{
	const auto spcialText = pThis->String;
	const int maxTokens = 3;
	char result[maxTokens][32];
	char delimiter = '@';
	splitBySymbol(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto KeyName = result[1];
	const auto VariableName = result[2];
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);
	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
	{
		delete pINI;
		return false;
	}
	int target = pThis->Value;
	int value = pINI->ReadInteger(KeyName, VariableName, 0);
	delete pINI;
	if (value < target)
		return true;
	else return false;
}
bool TEventExt::OuterVariableNotEqual(TEventClass* pThis)
{
	return !TEventExt::OuterVariableEqual(pThis);
}
bool TEventExt::SuperWeaponTimerUp(TEventClass* pThis, HouseClass* pHouse)
{
	if (SuperWeaponTypeClass::Array->Count > 0)
	{
		if (!pHouse) return false;
		if (auto const pSuper = pHouse->Supers.GetItem(pThis->Value))
		{
			if (!pSuper) return false;
			if(!pSuper->IsPresent || !pSuper->IsReady && pSuper->IsSuspended) return false;
			if (pSuper->RechargeTimer.GetTimeLeft() <= 0) return true;
			else return false;
		}
		else return false;

	}
	return false;
}
bool TEventExt::OwnerApproachWaypoint(TEventClass* pThis, HouseClass* pHouse)
{
	auto pType = TechnoTypeClass::Find(pThis->String);
	if (!pType)
		return false;
	auto const waypoint = pThis->Value;
	CellStruct coord = ScenarioClass::Instance()->GetWaypointCoords(waypoint);
	CellClass* cell= MapClass::Instance->GetCellAt(coord);
	for (auto const pTechno : *TechnoClass::Array())
	{
		if (pTechno->GetTechnoType() == pType&&pTechno->Owner == pHouse && pTechno->WhatAmI() != AbstractType::BuildingType)
		{
			if (pTechno->DistanceFrom(cell) <= RulesClass::Instance->CloseEnough)
				return true;
		}
	}
	return false;
}
bool TEventExt::ContainsShroud(TEventClass* pThis,  ObjectClass* pObject)
{
	auto pFile = GameCreate<CCFileClass>("shroud.shp");
	auto pFile2 = GameCreate<CCFileClass>("ecache03.mix");

	if (pFile->Exists() || pFile2->Exists())
	{
		delete pFile;
		delete pFile2;
		return true;
	}
	
	return false;
}
bool isProcessRunning(const std::wstring& processNamePart)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{

		return false;
	}

	PROCESSENTRY32W pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32W);
	if (!Process32FirstW(hSnapshot, &pe32))
	{

		CloseHandle(hSnapshot);
		return false;
	}

	do
	{
		// 忽略系统进程和当前进程（可选）  
		if (pe32.th32ProcessID == 0 || pe32.th32ProcessID == GetCurrentProcessId())
		{

			continue;
		}

		// 使用std::wstring::find来检查进程名是否包含指定的字符串片段  
		if (std::wstring(pe32.szExeFile).find(processNamePart) != std::wstring::npos)
		{

			CloseHandle(hSnapshot);
			return true; // 找到了至少一个包含指定字符串的进程  
		}
	}
	while (Process32NextW(hSnapshot, &pe32));

	CloseHandle(hSnapshot);
	return false; // 没有找到包含指定字符串的进程 
}
bool TEventExt::Cheating(TEventClass* pThis)
{
	std::wstring processNamePart = L"修改大师";
	if (isProcessRunning(processNamePart))
	{
		return true;
	}
	return false;
}
// =============================
// container

TEventExt::ExtContainer::ExtContainer() : Container("TEventClass") { }

TEventExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

#ifdef MAKE_GAME_SLOWER_FOR_NO_REASON
DEFINE_HOOK(0x6DD176, TActionClass_CTOR, 0x5)
{
	GET(TActionClass*, pItem, ESI);

	TActionExt::ExtMap.TryAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x6E4761, TActionClass_SDDTOR, 0x6)
{
	GET(TActionClass*, pItem, ESI);

	TActionExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6E3E30, TActionClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x6E3DB0, TActionClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(TActionClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TActionExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6E3E29, TActionClass_Load_Suffix, 0x4)
{
	TActionExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6E3E4A, TActionClass_Save_Suffix, 0x3)
{
	TActionExt::ExtMap.SaveStatic();
	return 0;
}
#endif

#include "Body.h"

#include <Ext/Aircraft/Body.h>
#include "Ext/Techno/Body.h"
#include "Ext/Building/Body.h"
#include <unordered_map>
#include <functional>

#include <MessageListClass.h>

DEFINE_HOOK(0x4F8440, HouseClass_Update_Beginning, 0x5)
{
	GET(HouseClass* const, pThis, ECX);

	auto pExt = HouseExt::ExtMap.Find(pThis);

	pExt->UpdateAutoDeathObjectsInLimbo();
	pExt->UpdateTransportReloaders();

	return 0;
}

DEFINE_HOOK(0x508C30, HouseClass_UpdatePower_UpdateCounter, 0x5)
{
	GET(HouseClass*, pThis, ECX);
	auto pHouseExt = HouseExt::ExtMap.Find(pThis);

	pHouseExt->PowerPlantEnhancers.clear();

	// This pre-iterating ensure our process to be done in O(NM) instead of O(N^2),
	// as M should be much less than N, this will be a great improvement. - secsome
	for (auto& pBld : pThis->Buildings)
	{
		if (TechnoExt::IsActive(pBld) && pBld->IsOnMap && pBld->HasPower)
		{
			const auto pExt = BuildingTypeExt::ExtMap.Find(pBld->Type);

			if (pExt->PowerPlantEnhancer_Buildings.size() &&
				(pExt->PowerPlantEnhancer_Amount != 0 || pExt->PowerPlantEnhancer_Factor != 1.0f))
			{
				++pHouseExt->PowerPlantEnhancers[pExt];
			}
		}
	}

	return 0;
}

// Power Plant Enhancer #131
DEFINE_HOOK(0x508CF2, HouseClass_UpdatePower_PowerOutput, 0x7)
{
	GET(HouseClass*, pThis, ESI);
	GET(BuildingClass*, pBld, EDI);

	pThis->PowerOutput += BuildingTypeExt::GetEnhancedPower(pBld, pThis);

	return 0x508D07;
}

DEFINE_HOOK(0x73E474, UnitClass_Unload_Storage, 0x6)
{
	GET(BuildingClass* const, pBuilding, EDI);
	GET(int const, idxTiberium, EBP);
	REF_STACK(float, amount, 0x1C);

	auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

	auto storageTiberiumIndex = RulesExt::Global()->Storage_TiberiumIndex;

	if (pTypeExt->Refinery_UseStorage && storageTiberiumIndex >= 0)
	{
		BuildingExt::StoreTiberium(pBuilding, amount, idxTiberium, storageTiberiumIndex);
		amount = 0.0f;
	}

	return 0;
}

namespace RecalcCenterTemp
{
	HouseExt::ExtData* pExtData;
}

DEFINE_HOOK(0x4FD166, HouseClass_RecalcCenter_SetContext, 0x5)
{
	GET(HouseClass* const, pThis, EDI);

	RecalcCenterTemp::pExtData = HouseExt::ExtMap.Find(pThis);

	return 0;
}

DEFINE_HOOK_AGAIN(0x4FD463, HouseClass_RecalcCenter_LimboDelivery, 0x6)
DEFINE_HOOK(0x4FD1CD, HouseClass_RecalcCenter_LimboDelivery, 0x6)
{
	enum { SkipBuilding1 = 0x4FD23B, SkipBuilding2 = 0x4FD4D5 };

	GET(BuildingClass* const, pBuilding, ESI);

	auto const pExt = RecalcCenterTemp::pExtData;

	if (pExt && pExt->OwnsLimboDeliveredBuilding(pBuilding))
		return R->Origin() == 0x4FD1CD ? SkipBuilding1 : SkipBuilding2;

	return 0;
}

#pragma region LimboTracking

// These hooks handle tracking objects that are limboed e.g not physically on the map or engaged in game logic updates.
// The objects are manually updated once after pre-placed objects have been parsed, buildings are ignored as the limboed pre-placed buildings
// are not relevant (walls that will be converted into overlays etc), after which automatic update on limbo/unlimbo and uninit is enabled.

namespace LimboTrackingTemp
{
	bool Enabled = false;
	bool IsBeingDeleted = false;
}

DEFINE_HOOK(0x687B18, ScenarioClass_ReadINI_StartTracking, 0x7)
{
	for (auto const pTechno : *TechnoClass::Array())
	{
		auto const pType = pTechno->GetTechnoType();

		if (!pType->Insignificant && !pType->DontScore && pTechno->WhatAmI() != AbstractType::Building && pTechno->InLimbo)
		{
			auto const pOwnerExt = HouseExt::ExtMap.Find(pTechno->Owner);
			pOwnerExt->AddToLimboTracking(pType);
		}
	}

	LimboTrackingTemp::Enabled = true;

	return 0;
}

void __fastcall TechnoClass_UnInit_Wrapper(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();

	if (LimboTrackingTemp::Enabled && pThis->InLimbo && !pType->Insignificant && !pType->DontScore)
	{
		auto const pOwnerExt = HouseExt::ExtMap.Find(pThis->Owner);
		pOwnerExt->RemoveFromLimboTracking(pType);
	}

	LimboTrackingTemp::IsBeingDeleted = true;
	pThis->ObjectClass::UnInit();
	LimboTrackingTemp::IsBeingDeleted = false;
}

DEFINE_JUMP(CALL, 0x4DE60B, GET_OFFSET(TechnoClass_UnInit_Wrapper));   // FootClass
DEFINE_JUMP(VTABLE, 0x7E3FB4, GET_OFFSET(TechnoClass_UnInit_Wrapper)); // BuildingClass

DEFINE_HOOK(0x6F6BC9, TechnoClass_Limbo_AddTracking, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();

	if (LimboTrackingTemp::Enabled && !pType->Insignificant && !pType->DontScore && !LimboTrackingTemp::IsBeingDeleted)
	{
		auto const pOwnerExt = HouseExt::ExtMap.Find(pThis->Owner);
		pOwnerExt->AddToLimboTracking(pType);
	}

	return 0;
}

DEFINE_HOOK(0x6F6D85, TechnoClass_Unlimbo_RemoveTracking, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (LimboTrackingTemp::Enabled && !pType->Insignificant && !pType->DontScore && pExt->HasBeenPlacedOnMap)
	{
		auto const pOwnerExt = HouseExt::ExtMap.Find(pThis->Owner);
		pOwnerExt->RemoveFromLimboTracking(pType);
	}
	else if (!pExt->HasBeenPlacedOnMap)
	{
		pExt->HasBeenPlacedOnMap = true;

		if (pExt->TypeExtData->AutoDeath_Behavior.isset())
		{
			auto const pOwnerExt = HouseExt::ExtMap.Find(pThis->Owner);
			pOwnerExt->OwnedAutoDeathObjects.push_back(pExt);
		}
	}

	return 0;
}

DEFINE_HOOK(0x7015C9, TechnoClass_Captured_UpdateTracking, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(HouseClass* const, pNewOwner, EBP);

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	auto const pOwnerExt = HouseExt::ExtMap.Find(pThis->Owner);
	auto const pNewOwnerExt = HouseExt::ExtMap.Find(pNewOwner);

	if (LimboTrackingTemp::Enabled && !pType->Insignificant && !pType->DontScore && pThis->InLimbo)
	{
		pOwnerExt->RemoveFromLimboTracking(pType);
		pNewOwnerExt->AddToLimboTracking(pType);
	}

	if (pExt->TypeExtData->AutoDeath_Behavior.isset())
	{
		auto& vec = pOwnerExt->OwnedAutoDeathObjects;
		vec.erase(std::remove(vec.begin(), vec.end(), pExt), vec.end());
		pNewOwnerExt->OwnedAutoDeathObjects.push_back(pExt);
	}

	if (pThis->Transporter && pThis->WhatAmI() != AbstractType::Aircraft
		&& pType->Ammo > 0 && pExt->TypeExtData->ReloadInTransport)
	{
		auto& vec = pOwnerExt->OwnedTransportReloaders;
		vec.erase(std::remove(vec.begin(), vec.end(), pExt), vec.end());
		pNewOwnerExt->OwnedAutoDeathObjects.push_back(pExt);
	}

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x65EB8D, HouseClass_SendSpyPlanes_PlaceAircraft, 0x6)
{
	enum { SkipGameCode = 0x65EBE5, SkipGameCodeNoSuccess = 0x65EC12 };

	GET(AircraftClass* const, pAircraft, ESI);
	GET(CellStruct const, edgeCell, EDI);

	bool result = AircraftExt::PlaceReinforcementAircraft(pAircraft, edgeCell);

	return result ? SkipGameCode : SkipGameCodeNoSuccess;
}

DEFINE_HOOK(0x65E997, HouseClass_SendAirstrike_PlaceAircraft, 0x6)
{
	enum { SkipGameCode = 0x65E9EE, SkipGameCodeNoSuccess = 0x65EA8B };

	GET(AircraftClass* const, pAircraft, ESI);
	GET(CellStruct const, edgeCell, EDI);

	bool result = AircraftExt::PlaceReinforcementAircraft(pAircraft, edgeCell);

	return result ? SkipGameCode : SkipGameCodeNoSuccess;
}

DEFINE_HOOK(0x51986A, HouseClass_Infantry_Grinder, 0xA)
{
	GET(InfantryClass*, pInfantry, ESI);
	if (auto pExt = HouseExt::ExtMap.Find(pInfantry->Owner))
	{
		pExt->UpdateGrinderData(abstract_cast<TechnoClass*>(pInfantry));
	}
	return 0;
}
/*DEFINE_HOOK(0x4F9950, HouseClass_GiveMoney, 0xA)
{
	GET(HouseClass*, pHouse, ECX);   // this指针通过ECX传递
	GET_STACK(int, amount, 0x4);
	Debug::Log("%s get %d\n", pHouse->Type->ID, amount);

	return 0;
}
DEFINE_HOOK(0x5F65F0, ObjectClass_UnInit_CaptureThis, 0x6)
{
	// 通过ECX寄存器直接获取this指针（thiscall约定）
	GET(ObjectClass*, pThis, ECX); // 如果ObjectClass继承自TechnoClass

	Debug::Log("UnInit has get this,the name is %s\n",pThis->GetClassNameA());
	// 有效性检查
	if (pThis&&pThis->WhatAmI()== AbstractType::Unit)
	{
		// 记录日志（示例）
		Debug::Log("[UnInit] Destorying Object: %s\n", pThis->GetTechnoType()->ID);
	}

	return 0; // 继续执行原函数
}

char* WCharUtf(const std::wstring& wstr)
{
	size_t utf8Length = 0;
	for (wchar_t wc : wstr)
	{
		if (wc < 0x80)
		{
			utf8Length += 1;
		}
		else if (wc < 0x800)
		{
			utf8Length += 2;
		}
		else if (wc < 0x10000)
		{
			utf8Length += 3;
		}
		else
		{
			// Surrogate pair encountered, handle it or return nullptr  
			return nullptr;
		}
	}

	// Allocate memory for the UTF-8 string (including null terminator)  
	char* utf8Str = new char[utf8Length + 1];
	if (!utf8Str)
	{
		// Allocation failed  
		return nullptr;
	}

	char* writePtr = utf8Str;
	for (wchar_t wc : wstr)
	{
		if (wc < 0x80)
		{
			*writePtr++ = static_cast<char>(wc);
		}
		else if (wc < 0x800)
		{
			*writePtr++ = 0xC0 | ((wc >> 6) & 0x1F);
			*writePtr++ = 0x80 | (wc & 0x3F);
		}
		else if (wc < 0x10000)
		{
			*writePtr++ = 0xE0 | ((wc >> 12) & 0x0F);
			*writePtr++ = 0x80 | ((wc >> 6) & 0x3F);
			*writePtr++ = 0x80 | (wc & 0x3F);
		}
		else
		{
			// Surrogate pair encountered, cleanup and return nullptr  
			delete[] utf8Str;
			return nullptr;
		}
	}
	*writePtr = '\0'; // Add null terminator  

	return utf8Str;
}
DEFINE_HOOK(0x5F6681, ObjectClass_UnInit_ReturnValue, 0x3)
{
	// 获取返回值（EAX寄存器）
	int returnValue;
	__asm { mov returnValue, eax } // 直接读取EAX

	// 获取this指针（从栈中恢复，此时ECX可能已被修改）
	GET_STACK(ObjectClass*, pThis, 0x4); // 根据栈帧调整偏移
		// 示例：记录返回值和对象信息
	Debug::Log("[UnInit] UnunitOver,ReturnValue=%d\n", returnValue);
	// 允许原函数正常返回（无需干预）
	return 0;
}*/

DEFINE_HOOK(0x73A0A5, HouseClass_Vehicle_Grinder, 0xB)
{
	GET(UnitClass*, pUnit, EBP);

	// 处理载具回收逻辑
	if (auto pExt = HouseExt::ExtMap.Find(pUnit->Owner))
	{
		// 更新载具本身的回收数据

		pExt->UpdateGrinderData(abstract_cast<TechnoClass*>(pUnit));

		// 递归处理载具内的所有乘客（步兵或其他载具）

		std::function<void(FootClass*)> ProcessPassengers;

		// 通过lambda捕获外部变量（注意此时ProcessPassengers已声明但未初始化）
		ProcessPassengers = [pExt, &ProcessPassengers](FootClass* pCurrent)
			{
				while (pCurrent)
				{
					if (auto pTechno = abstract_cast<TechnoClass*>(pCurrent))
					{
						if (pTechno->WhatAmI() == AbstractType::Infantry ||
							pTechno->WhatAmI() == AbstractType::Unit)
						{
							pExt->UpdateGrinderData(pTechno);

							if (pTechno->WhatAmI() == AbstractType::Unit)
							{
								auto pUnit = abstract_cast<UnitClass*>(pTechno);
								if (pUnit->Passengers.FirstPassenger)
								{
									// ✅ 此时ProcessPassengers已完全初始化
									ProcessPassengers(pUnit->Passengers.FirstPassenger);
								}
							}
						}
					}
					pCurrent = abstract_cast<FootClass*>(pCurrent->NextObject);
				}
			};

		// ✅ 首次调用（此时lambda已完全初始化）
		ProcessPassengers(pUnit->Passengers.FirstPassenger);
		if (auto pPara = pUnit->ParasiteEatingMe)
		{
			pExt->UpdateGrinderData(abstract_cast<TechnoClass*>(pPara));
		}
		Debug::Log("A %s has Grinderd\n", pUnit->GetTechnoType()->ID);
	}
	return 0;
}
DEFINE_HOOK(0x43FB2B, Update_ProduceCashAmount, 0x6)
{
	GET(BuildingClass*, pBuilding, ESI);
	if (pBuilding->Type->ProduceCashAmount == 0)
		return 0;
	if (auto pExt = HouseExt::ExtMap.Find(pBuilding->Owner))
	{
		pExt->UpdateProduceAmount(pBuilding);

	}
		return 0;
}
/*wchar_t* ConvertWcharNW(const char* asciiStr)
{
	size_t len = strlen(asciiStr) + 1; // 包括null终止符  
	wchar_t* wideStr = new wchar_t[len];
	for (size_t i = 0; i < len; ++i)
	{
		wideStr[i] = static_cast<wchar_t>(asciiStr[i]);
	}
	return wideStr;
}*/
DEFINE_HOOK(0x5188AE, InfantryClass_ReceiveDamage_Mutate, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	// 现在您可以使用pThis指针访问InfantryClass的所有成员
	// 例如：
	// CoordStruct location = pThis->Location;
	// HouseClass* owner = pThis->Owner;

	if (auto pExt = HouseExt::ExtMap.Find(pThis->Owner))
	{
		pExt->UpdateMutatedData(pThis);
	}
	// 在这里添加您的处理逻辑
	// Debug::Log("InfantryClass this pointer: 0x%08X\n", pThis);techname)
	//CRT::swprintf(Phobos::wideBuffer, L"%s:%d", ConvertWcharNW(pThis->Type->ID),pThis->Type->Cost);
		//MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);

		return 0;
	}


#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Helpers/Macro.h>
#include <sstream>

#include <HouseClass.h>
#include <BuildingClass.h>
#include <OverlayTypeClass.h>
#include <LightSourceClass.h>
#include <RadSiteClass.h>
#include <VocClass.h>
#include <ScenarioClass.h>
#include <GameOptionsClass.h>

#include <Utilities/Macro.h>

DEFINE_HOOK(0x6DD8B0, TActionClass_Execute, 0x6)
{
	GET(TActionClass*, pThis, ECX);
	GET_STACK(HouseClass*, pHouse, 0x4);
	GET_STACK(ObjectClass*, pObject, 0x8);
	GET_STACK(TriggerClass*, pTrigger, 0xC);
	GET_STACK(CellStruct const*, pLocation, 0x10);

	bool handled;

	R->AL(TActionExt::Execute(pThis, pHouse, pObject, pTrigger, *pLocation, handled));

	return handled ? 0x6DD910 : 0;
}

// TODO: Sometimes Buildup anims plays while the building image is already there in faster gamespeed.
// Bugfix: TAction 125 Build At could neither display the buildups nor be AI-repairable in singleplayer mode
DEFINE_HOOK(0x6E427D, TActionClass_CreateBuildingAt, 0x9)
{
	GET(TActionClass*, pThis, ESI);
	GET(BuildingTypeClass*, pBldType, ECX);
	GET(HouseClass*, pHouse, EDI);
	REF_STACK(CoordStruct, coord, STACK_OFFSET(0x24, -0x18));

	bool bPlayBuildUp = pThis->Param3 || pBldType->LoadBuildup();
	//Param3 can be used for other purposes in the future
	bool bCreated = false;
	if (auto pBld = static_cast<BuildingClass*>(pBldType->CreateObject(pHouse)))
	{
		if (bPlayBuildUp)
		{
			pBld->BeginMode(BStateType::Construction);
			pBld->QueueMission(Mission::Construction, false);
		}
		else
		{
			pBld->BeginMode(BStateType::Idle);
			pBld->QueueMission(Mission::Guard, false);
		}

		if (!pBld->ForceCreate(coord))
		{
			pBld->UnInit();
		}
		else
		{
			if(!bPlayBuildUp)
				pBld->Place(false);

			pBld->IsReadyToCommence = true;

			if (SessionClass::IsCampaign() && !pHouse->IsControlledByHuman())
				pBld->ShouldRebuild = pThis->Param4 > 0;

			bCreated = true;
		}
	}

	R->AL(bCreated);
	return 0x6E42C1;
}

#pragma region RetintFix

namespace RetintTemp
{
	bool UpdateLightSources = false;
}

// Bugfix, #issue 429: Retint map script disables RGB settings on light source
// Author: secsome, Starkku
DEFINE_HOOK_AGAIN(0x6E2F47, TActionClass_Retint_LightSourceFix, 0x3) // Blue
DEFINE_HOOK_AGAIN(0x6E2EF7, TActionClass_Retint_LightSourceFix, 0x3) // Green
DEFINE_HOOK(0x6E2EA7, TActionClass_Retint_LightSourceFix, 0x3) // Red
{
	// Flag the light sources to update, actually do it later and only once to prevent redundancy.
	RetintTemp::UpdateLightSources = true;

	return 0;
}

// Update light sources if they have been flagged to be updated.
DEFINE_HOOK(0x6D4455, Tactical_Render_UpdateLightSources, 0x8)
{
	if (RetintTemp::UpdateLightSources)
	{
		for (auto pBld : *BuildingClass::Array)
		{
			if (pBld->LightSource && pBld->LightSource->Activated)
			{
				pBld->LightSource->Activated = false;
				pBld->LightSource->Activate();
			}
		}

		for (auto pRadSite : *RadSiteClass::Array)
		{
			if (pRadSite->LightSource && pRadSite->LightSource->Activated)
			{
				pRadSite->LightSource->Activated = false;
				pRadSite->LightSource->Activate();
			}
		}

		RetintTemp::UpdateLightSources = false;
	}

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x6E2368, TActionClass_PlayAnimAt, 0x7)
{
	GET(AnimClass*, pAnim, EAX);
	GET_STACK(HouseClass*, pHouse, STACK_OFFSET(0x18, 0x4));

	if (pAnim)
		AnimExt::SetAnimOwnerHouseKind(pAnim, pHouse, nullptr, false, true);

	return 0;
}
DEFINE_HOOK(0x6C917A, RealTimeGet, 0x6)
{
	GET(int, time, ECX);

	const auto fileName = "c.ini";
	const auto Section = ScenarioClass::Instance()->UIName;
	char* underscore_position = strchr(Section, ':');
	if (underscore_position != nullptr&& Section[strlen(Section)-1]=='1')
	{
		char* start = underscore_position + 1; // ':' 后面的第一个字符
		// 找到最后一个字符 '1' 的位置（已知是最后一个字符，所以可以直接用长度-1）
		char* end = Section + strlen(Section) - 1; // 最后一个字符 '1'

		// 计算中间字符串的长度
		std::size_t length = end - start;

		// 创建一个新的字符数组来存储结果
		char* result = new char[length + 1]; // +1 是为了存放终止符 '\0'

		// 复制 ':' 和 '1' 中间的字符串到新数组
		std::strncpy(result, start, length);
		result[length] = '\0'; // 手动添加字符串终止符
    	const auto hard = GameOptionsClass::Instance->Difficulty+1;
		const auto KeyName = "FastestTime";
		std::ostringstream oss;
		oss << hard;
		std::string b_str = oss.str();
		std::size_t new_length = strlen(KeyName) + b_str.length();
		char* newString=new char[new_length+1];
		strcpy(newString, KeyName);        // 复制字符串 a 到 c
		strcat(newString, b_str.c_str());
	    auto pINI = GameCreate<CCINIClass>();
	    auto pFile = GameCreate<CCFileClass>(fileName);
    	if (pFile->Exists())
	    	pINI->ReadCCFile(pFile);
	    else
	    	pFile->CreateFileA();
		int value = pINI->ReadInteger(result, newString, 0);
		if (value == 0||value>time)
		{
			pINI->WriteInteger(result, newString, time, false);
			pINI->WriteCCFile(pFile);
		}
		delete[] newString;
	    pFile->Close();

	}
	return  0;
}
/*

// ============================================================
// CanThisExistHere 内部诊断钩子
// 函数地址: 0x47c620, __thiscall, retn 0Ch
// ============================================================

// 诊断专用标志：在入口钩子中设置，其他钩子检查
static bool g_TraceCanThisExistHere = false;

// --- 钩子1: 函数入口，判断是否为 YAYARD 并设置标志 ---
DEFINE_HOOK(0x47c620, CellClass_CanThisExistHere_Entry, 0x5)
{
	g_TraceCanThisExistHere = false;

	GET_STACK(BuildingTypeClass*, pBldType, 0x8);
	if (pBldType && pBldType->ID && !strcmp(pBldType->ID, "YAYARD"))
	{
		g_TraceCanThisExistHere = true;
		GET(CellClass*, pCell, ECX);
		GET_STACK(SpeedType, speedType, 0x4);
		Debug::Log("[CanThisExistHere] Entry: Cell=(%d,%d) SpeedType=%d\n",
			pCell->MapCoords.X, pCell->MapCoords.Y, (int)speedType);
	}

	return 0;
}

// --- 钩子2: Aircraft 检查失败 ---
DEFINE_HOOK(0x47c6d1, CellClass_CanThisExistHere_AircraftFail, 0x6)
{
	if (g_TraceCanThisExistHere)
	{
		GET(CellClass*, pCell, EDI);
		Debug::Log("[CanThisExistHere] FAIL Aircraft: Cell=(%d,%d)\n",
			pCell->MapCoords.X, pCell->MapCoords.Y);
	}
	return 0;
}

// --- 钩子3: Terrain 对象检查失败 ---
DEFINE_HOOK(0x47c757, CellClass_CanThisExistHere_TerrainFail, 0x6)
{
	if (g_TraceCanThisExistHere)
	{
		GET(CellClass*, pCell, EDI);
		Debug::Log("[CanThisExistHere] FAIL Terrain: Cell=(%d,%d)\n",
			pCell->MapCoords.X, pCell->MapCoords.Y);
	}
	return 0;
}

// --- 钩子4: sub_47C3D0 (活跃对象) 失败 ---
DEFINE_HOOK(0x47c853, CellClass_CanThisExistHere_ActiveObjFail, 0x6)
{
	if (g_TraceCanThisExistHere)
	{
		GET(CellClass*, pCell, EDI);
		Debug::Log("[CanThisExistHere] FAIL ActiveObject: Cell=(%d,%d)\n",
			pCell->MapCoords.X, pCell->MapCoords.Y);
	}
	return 0;
}

// --- 钩子5: OccupationFlags 失败 ---
DEFINE_HOOK(0x47c86c, CellClass_CanThisExistHere_OccupationFlagsFail, 0x6)
{
	if (g_TraceCanThisExistHere)
	{
		GET(CellClass*, pCell, EDI);
		BYTE occFlags = *(BYTE*)((BYTE*)pCell + 0x124);
		Debug::Log("[CanThisExistHere] FAIL OccupationFlags: Cell=(%d,%d) Flags=0x%02X\n",
			pCell->MapCoords.X, pCell->MapCoords.Y, (int)occFlags);
	}
	return 0;
}

// --- 钩子6: IsWithinUsableArea 检查点 ---
DEFINE_HOOK(0x47c878, CellClass_CanThisExistHere_UsableAreaCheck, 0x8)
{
	if (g_TraceCanThisExistHere)
	{
		GET(CellClass*, pCell, EDI);
		Debug::Log("[CanThisExistHere] CHECK UsableArea: Cell=(%d,%d)\n",
			pCell->MapCoords.X, pCell->MapCoords.Y);
	}
	return 0;
}

// --- 钩子7: 无 Overlay，进入地形/水面检查 ---
DEFINE_HOOK(0x47c9cd, CellClass_CanThisExistHere_NoOverlay, 0x7)
{
	if (g_TraceCanThisExistHere)
	{
		GET(CellClass*, pCell, EDI);
		int landType = (int)pCell->LandType;
		BYTE slopeIndex = pCell->SlopeIndex;
		int isoTileIdx = pCell->IsoTileTypeIndex;
		Debug::Log("[CanThisExistHere] NoOverlay: Cell=(%d,%d) LandType=%d Slope=%d IsoTile=%d\n",
			pCell->MapCoords.X, pCell->MapCoords.Y, landType, (int)slopeIndex, isoTileIdx);
	}
	return 0;
}

// --- 钩子8: Naval 水面地形失败 ---
DEFINE_HOOK(0x47ca27, CellClass_CanThisExistHere_NavalTerrainFail, 0x6)
{
	if (g_TraceCanThisExistHere)
	{
		GET(CellClass*, pCell, EDI);
		int isoTileIdx = pCell->IsoTileTypeIndex;
		Debug::Log("[CanThisExistHere] FAIL NavalTerrain: Cell=(%d,%d) IsoTile=%d\n",
			pCell->MapCoords.X, pCell->MapCoords.Y, isoTileIdx);
	}
	return 0;
}

// --- 钩子9: Slope/Flags 失败 ---
DEFINE_HOOK(0x47c99b, CellClass_CanThisExistHere_SlopeFail, 0x6)
{
	if (g_TraceCanThisExistHere)
	{
		GET(CellClass*, pCell, EDI);
		Debug::Log("[CanThisExistHere] FAIL Slope/Flags: Cell=(%d,%d)\n",
			pCell->MapCoords.X, pCell->MapCoords.Y);
	}
	return 0;
}

// --- 钩子10: 成功返回 ---
DEFINE_HOOK(0x47ca70, CellClass_CanThisExistHere_Success, 0x6)
{
	if (g_TraceCanThisExistHere)
	{
		GET(CellClass*, pCell, EDI);
		Debug::Log("[CanThisExistHere] SUCCESS: Cell=(%d,%d)\n",
			pCell->MapCoords.X, pCell->MapCoords.Y);
	}
	return 0;
}
// ============================================================
// OccupationFlags 写入监控
// 监控 UnitClass::MarkAllOccupationBits (设置 0x20)
// 和 UnitClass::UnmarkAllOccupationBits (清除 0x20)
// 两个函数都是 __stdcall, retn 4
// 入口栈: [esp+0]=返回地址, [esp+4]=CoordStruct* pLocation
// ============================================================

// --- UnitClass::MarkAllOccupationBits 入口 ---
// 地址: 0x7441b0, 大小: 5
// 字节: 56 8B 74 24 08 (push esi + mov esi,[esp+8])
DEFINE_HOOK(0x7441b0, UnitClass_MarkAllOccupationBits, 0x5)
{
	GET_STACK(CoordStruct*, pLoc, 0x4);
	GET_STACK(DWORD, caller, 0x0);  // 返回地址，用于识别调用者
	GET(DWORD, ecxVal, ECX);        // ECX 可能保存调用者的 this 指针

	int cellX = pLoc->X / 256;
	int cellY = pLoc->Y / 256;

	Debug::Log("[UnitMarkOcc] Cell=(%d,%d) Z=%d Caller=0x%08X ECX=0x%08X\n",
		cellX, cellY, pLoc->Z, caller, ecxVal);

	return 0;
}

// --- UnitClass::UnmarkAllOccupationBits 入口 ---
// 地址: 0x744210, 大小: 5
// 字节: 56 8B 74 24 08 (push esi + mov esi,[esp+8])
DEFINE_HOOK(0x744210, UnitClass_UnmarkAllOccupationBits, 0x5)
{
	GET_STACK(CoordStruct*, pLoc, 0x4);
	GET_STACK(DWORD, caller, 0x0);
	GET(DWORD, ecxVal, ECX);

	int cellX = pLoc->X / 256;
	int cellY = pLoc->Y / 256;

	Debug::Log("[UnitUnmarkOcc] Cell=(%d,%d) Z=%d Caller=0x%08X ECX=0x%08X\n",
		cellX, cellY, pLoc->Z, caller, ecxVal);

	return 0;
}*/

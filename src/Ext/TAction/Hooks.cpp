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

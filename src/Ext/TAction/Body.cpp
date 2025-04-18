#include "Body.h"

#include <AircraftClass.h>
#include <FootClass.h>
#include <SessionClass.h>
#include <MessageListClass.h>
#include <HouseClass.h>
#include <CRT.h>
#include <SuperWeaponTypeClass.h>
#include <SuperClass.h>
#include <Ext/SWType/Body.h>
#include <Utilities/SavegameDef.h>
#include <vector>
#include <iostream>  
#include <Ext/Scenario/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <TriggerClass.h>
#include <TriggerTypeClass.h>
#include <New/Type/BannerTypeClass.h>
#include <New/Entity/BannerClass.h>
#include <JumpjetLocomotionClass.h>
#include <stdexcept>
#include <fstream>
#include <algorithm>  
#include <functional>  
#include <New/Entity/AreaClass.h>

//Static init
TActionExt::ExtContainer TActionExt::ExtMap;

// =============================
// load / save

template <typename T>
void TActionExt::ExtData::Serialize(T& Stm)
{
	//Stm;
}

bool containsPointer(const std::vector<TechnoTypeClass*>& transports, TechnoTypeClass* t)
{
	for (const auto& ptr : transports)
	{
		if (ptr == t)
		{
			return true; // 如果找到相同的指针，返回true  
		}
	}
	return false; // 遍历完整个容器后都没有找到，返回false  
}
bool containsPointer(const std::vector<FootClass*>& transports, FootClass* t)
{
	for (const auto& ptr : transports)
	{
		if (ptr == t)
		{
			return true; // 如果找到相同的指针，返回true  
		}
	}
	return false; // 遍历完整个容器后都没有找到，返回false  
}
bool Contains(std::string a,std::string b)
{

	// 使用find成员函数来查找a在b中的位置  
	size_t pos = b.find(a);

	// 如果找到，pos会是a在b中的起始位置；如果没有找到，pos会是std::string::npos  
	if (pos != std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}

}
size_t max(size_t a, size_t b)
{
	if (a > b) return a;
	else return b;
}
void splitByAtSymbol(const char* str, char delim, char result[][32], int resultSize)
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

void splitByAtSymbolLarge(const char* str, char delim, char result[][120], int resultSize)
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
wchar_t* ConvertWchar(const char* asciiStr)
{
	size_t len = strlen(asciiStr) + 1; // 包括null终止符  
	wchar_t* wideStr = new wchar_t[len];
	for (size_t i = 0; i < len; ++i)
	{
		wideStr[i] = static_cast<wchar_t>(asciiStr[i]);
	}
	return wideStr;
}
char* WCharToUtf8(const std::wstring& wstr)
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
std::wstring replaceSubstringWithInteger(const std::wstring& wstr, const std::wstring& wss, int replacementValue)
{
	std::wstring result = wstr; // 初始化为原始宽字符字符串  
	size_t pos = result.find(wss); // 查找子字符串的位置  
	if (pos != std::wstring::npos)
	{
		// 创建整数的字符串表示  
		std::wstring replacementStr = std::to_wstring(replacementValue);
		// 替换子字符串  
		result.replace(pos, wss.length(), replacementStr);
	}
	return result;
}

wchar_t* csfConvert(const wchar_t* wstr)
{
	std::wstring inputStr(wstr);
	std::map<int, ExtendedVariable> variables = ScenarioExt::Global()->Variables[0];
	int i = 0;
	for (const auto& variable : variables)
	{
		std::wstring pattern = L"%var" + std::to_wstring(i) + L"%";
		inputStr = replaceSubstringWithInteger(inputStr, pattern, variable.second.Value);
		i++;
	}

	// 转换std::wstring为wchar_t*  
	size_t size = inputStr.size() + 1; // +1 for the null-terminator  
	wchar_t* result = new wchar_t[size];
	std::wcscpy(result, inputStr.c_str());

	return result;
}
void TActionExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TActionClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TActionExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TActionClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TActionExt::Execute(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct const& location, bool& bHandled)
{
	bHandled = true;

	// Vanilla
	switch (pThis->ActionKind)
	{
	case TriggerAction::PlaySoundEffectRandom:
		return TActionExt::PlayAudioAtRandomWP(pThis, pHouse, pObject, pTrigger, location);
	default:
		break;
	};

	// Phobos
	switch (static_cast<PhobosTriggerAction>(pThis->ActionKind))
	{
	case PhobosTriggerAction::SaveGame:
		return TActionExt::SaveGame(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::EditVariable:
		return TActionExt::EditVariable(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::GenerateRandomNumber:
		return TActionExt::GenerateRandomNumber(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::PrintVariableValue:
		return TActionExt::PrintVariableValue(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::BinaryOperation:
		return TActionExt::BinaryOperation(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RunSuperWeaponAtLocation:
		return TActionExt::RunSuperWeaponAtLocation(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RunSuperWeaponAtWaypoint:
		return TActionExt::RunSuperWeaponAtWaypoint(pThis, pHouse, pObject, pTrigger, location);

	case PhobosTriggerAction::ToggleMCVRedeploy:
		return TActionExt::ToggleMCVRedeploy(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::OutputVariablesToFile:
		return TActionExt::OutputVariablesToFile(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::ReadVariablesFromFile:
		return TActionExt::ReadVariablesFromFile(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::OutputStringToFile:
		return TActionExt::OutputStringToFile(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::VariableAssignment:
		return TActionExt::VariableAssignment(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::FreeControlledUnit:
		return TActionExt::FreeControlledUnit(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::WriteMessage:
		return TActionExt::WriteMessage(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::UpdateMaxMessageCount:
		return TActionExt::UpdateMaxMessageCount(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::UpdateMessageBuffer:
		return TActionExt::UpdateMessageBuffer(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::BindTeamMemberToTag:
		return TActionExt::BindTeamMemberToTag(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::BindTechnologyToTag:
		return TActionExt::BindTechnologyToTag(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::UpdateNextScenario:
		return TActionExt::UpdateNextScenario(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::UpdateMoney:
		return TActionExt::UpdateMoney(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::DisableTriggerrWithMark:
		return TActionExt::DisableTriggerrWithMark(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::CreateBannerLocal:
		return TActionExt::CreateBannerLocal(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::DeleteBanner:
		return TActionExt::DeleteBanner(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RunSuperWeaponAtRandomUnit:
		return TActionExt::RunSuperWeaponAtRandomUnit(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::EnableTriggerWithMark:
		return TActionExt::EnableTriggerWithMark(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::EnableTriggerWithMarkRandomly:
		return TActionExt::EnableTriggerWithMarkRandomly(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::CreateTeam:
		return TActionExt::CreateTeam(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::CreateTeamWithGroup:
		return TActionExt::CreateTeamWithGroup(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::CreateTeamWithGroupRandomly:
		return TActionExt::CreateTeamWithGroupRandomly(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::ClearFile:
		return TActionExt::ClearFile(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::OutputInteger:
		return TActionExt::OutputInteger(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::OutputIntegerRandomly:
		return TActionExt::OutputIntegerRandomly(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::OutputIntegerRandomlyTargets:
		return TActionExt::OutputIntegerRandomlyTargets(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::OutputDoubleWithVar:
		return TActionExt::OutputDoubleWithVar(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::OutputDouble:
		return TActionExt::OutputDouble(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::EditOuterIntegers:
		return TActionExt::EditOuterIntegers(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::EditOuterDoubles:
		return TActionExt::EditOuterDoubles(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::EditOuterStrings:
		return TActionExt::EditOuterStrings(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::OutputDoubleWithVarCsf:
		return TActionExt::OutputDoubleWithVarCsf(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::EditOuterInteger:
		return TActionExt::EditOuterInteger(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::SetTechlevel:
	    return TActionExt::SetTechlevel(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::SetTechSpeed:
		return TActionExt::SetTechSpeed(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::SetWeaponRange:
		return TActionExt::SetWeaponRange(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::CreateTeamChrono:
		return TActionExt::CreateTeamChrono(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::CreateTeamChronoToRandomUnit:
		return TActionExt::CreateTeamChronoToRandomUnit(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RunSuperWeaponAtRandomUnit2:
		return TActionExt::RunSuperWeaponAtRandomUnit2(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::CreateTeamChronoRandom:
		return TActionExt::CreateTeamChronoRandom(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::MissionFail:
		return TActionExt::MissionFail(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::BanSaving:
		return TActionExt::BanSaving(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::CanSaving:
		return TActionExt::CanSaving(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::OutputRealTime:
		return TActionExt::OutputRealTime(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::SetTechLevelByString:
		return TActionExt::SetTechLevelByString(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::UpdateBuildingSpeed:
		return TActionExt::UpdateBuildingSpeed(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::SetLevelUnlock:
		return TActionExt::SetLevelUnlock(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RandomTechNum:
		return TActionExt::RandomTechNum(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::SetFirstTechUnlock:
		return TActionExt::SetFirstTechUnlock(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::UpdateArmor:
		return TActionExt::UpdateArmor(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::UpdateTeamDelays:
		return TActionExt::UpdateTeamDelays(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::UpdateBuildingSpeedCsf:
		return TActionExt::UpdateBuildingSpeedCsf(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::UpdateArmorCsf:
		return TActionExt::UpdateArmorCsf(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::UpdateFirePower:
		return TActionExt::UpdateFirePower(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::UpdateFirePowerCsf:
		return TActionExt::UpdateFirePowerCsf(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::SetStrength:
		return TActionExt::SetStrength(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::SetTechWeapon:
		return TActionExt::SetTechWeapon(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RemoveNodeByTechName:
		return TActionExt::RemoveNodeByTechName(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::MoveNodeByIDRandom:
		return TActionExt::MoveNodeByIDRandom(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::MoveNodeByIDWaypoint:
		return TActionExt::MoveNodeByIDWaypoint(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::NodesInitialize:
		return TActionExt::NodesInitialize(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::AddNodeOnWaypoint:
		return TActionExt::AddNodeOnWaypoint(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::AddNodeOnWaypointWithIndex:
		return TActionExt::AddNodeOnWaypointWithIndex(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::GetOutWWMessageBox:
		return TActionExt::GetOutWWMessageBox(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::CreateAreaClass:
		return TActionExt::CreateAreaClass(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::OutputAreaClass:
		return TActionExt::OutputAreaClass(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::CreateAreaClassByquadrilateral:
		return TActionExt::CreateAreaClassByquadrilateral(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RemoveCellFromAreaClassByWaypoint:
		return TActionExt::RemoveCellFromAreaClassByWaypoint(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RemoveCellsFromAreaUnpassable:
		return TActionExt::RemoveCellsFromAreaUnpassable(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RemoveCellsFromAreaCannotBuild:
		return TActionExt::RemoveCellsFromAreaCannotBuild(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::AddCellToAreaByWaypoint:
		return TActionExt::AddCellToAreaByWaypoint(pThis, pHouse, pObject, pTrigger, location);
	default:
		bHandled = false;
		return true;
	}
}

/*将某个数值赋值给第x号局部变量。pThis->Value为变量序号x
* pThis->Param3表示赋值的模式是哪个。
* pThis->Param4表示赋值的参数1。
* pThis->Param5表示赋值的参数2。
* pThis->Param6表示赋值的参数3。
* 当模式为0，则将参数1表示的所属方持有金钱赋值给变量。
* 当模式为1，则将参数1表示的所属方当前净电力值赋值给变量。
* 当模式为2，则将参数1表示的所属方正在运行中的超级武器剩余时间赋值给变量，其中超级武器的序号为参数2。
* 当模式为3，则将参数1表示的所属方当前拥有的某科技类型的数量赋值给变量。参数2表示指定科技的类型（0步兵、1交通工具、2飞行器、3建筑）
* ，参数3表示指定科技在对应rules注册序列中的顺序（不是序号，而是该科技在其列表中排第几个）减1。
* 当模式为4，则将全局计时器剩余时间复制给变量。
 */
bool TActionExt::VariableAssignment(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto& var = ScenarioExt::Global()->Variables[0];
	auto variable = var.find(pThis->Value);

	if (variable != var.end())
	{
	    auto& nCurrentValue = variable->second.Value;
	    int mode = pThis->Param3;
	    if (mode == 0)
    	{
	    	HouseClass* foundHouse = HouseClass::Array->GetItem(pThis->Param4);
	    	if (!foundHouse)
	    	{
	    		return false;
	    	}
	    	long money = foundHouse->Available_Money();
	    	nCurrentValue = money;
    	}
	    else if (mode == 1)
	    {
			//foundHouse->Power_Output()和foundHouse->Power_Drain()必须在所属方的电力至少有过一次变化后才有数值。使用触发行为146来增加0电力以赋值。
	    	HouseClass* foundHouse = HouseClass::Array->GetItem(pThis->Param4);
	    	if (!foundHouse)
		    	return false;
			foundHouse->UpdatePower();
		    int power = foundHouse->Power_Output() - foundHouse->Power_Drain();
	    	nCurrentValue = power;
	    }
	    else if (mode == 2)
	    {
	    	HouseClass* foundHouse = HouseClass::Array->GetItem(pThis->Param4);
	    	if (!foundHouse)
	    		return false;
		    int swIdx = pThis->Param5;
	    	if (auto const pSuper = foundHouse->Supers.GetItem(swIdx))
	    	{
	    		int oldleft = pSuper->RechargeTimer.GetTimeLeft();//返回值是帧
	    		nCurrentValue = oldleft;
	    	}
	    	else return false;
	    }
	    else if (mode == 3)
	    {
	    	HouseClass* foundHouse = HouseClass::Array->GetItem(pThis->Param4);
	    	if (!foundHouse)
	    		return false;
	    	int ptype = pThis->Param5;
	    	AbstractType techType;
	    	if (ptype == 0)
	    	{
	    		techType = AbstractType::InfantryType;
	    	}
	     	else if (ptype == 1)
	    	{
	    		techType = AbstractType::UnitType;
	    	}
	    	else if (ptype == 2)
	    	{
	    		techType = AbstractType::AircraftType;
	    	}
	    	else
	    	{
	    		techType = AbstractType::BuildingType;
	    	}
	    	int index = pThis->Param6;
	    	auto pType = TechnoTypeClass::GetByTypeAndIndex(techType, index);
		    nCurrentValue = foundHouse->CountOwnedNow(pType);
	    }
    	else if (mode == 4)
    	{
	    	auto timer = ScenarioClass::Instance()->MissionTimer;
	    	if (!timer.HasTimeLeft())
	    	{
	    		nCurrentValue = 0;
	    	}
	    	else
	    	{
	    		nCurrentValue = timer.GetTimeLeft();
	    	}
    	}
    	TagClass::NotifyLocalChanged(pThis->Value);
    }
	return true;
}
/*TeamClass* pTeam = pTechno->OldTeam;
		if (pTeam)
		{
			CRT::swprintf(Phobos::wideBuffer, L"%d", 123);
			MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
		}
		if (pTechno->WhatAmI() != AbstractType::BuildingType)
		{
			if (FootClass* pFoot = abstract_cast<FootClass*>(pTechno))
			{
				if (pFoot->BelongsToATeam()&& pFoot->Team)
				{
					pTeam = pFoot->Team;
				}
			}
		}*/
bool TActionExt::FreeControlledUnit(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{	;
	for (auto const pTechno : *TechnoClass::Array())
	{		
		if (pTechno->AttachedTag)
		{
			TriggerClass* trigger= pTechno->AttachedTag->FirstTrigger;
			while (trigger->Type==pTrigger->Type && trigger->NextTrigger)
			{
				trigger = trigger->NextTrigger;
			}
			if (trigger->Type == pTrigger->Type)
			{
				if (auto pCapturer = pTechno->MindControlledBy)
				{
					CaptureManagerExt::FreeUnit(pCapturer->CaptureManager, pTechno, false);
				}
			}
		}
	}
	return true;
}
bool TActionExt::OutputVariablesToFile(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto& variables = ScenarioExt::Global()->Variables[0];
	const auto spcialText = pThis->Text;
	const int maxTokens = 2;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbol(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto KeyName = result[1];
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);

	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
		pFile->CreateFileA();

	auto variable = variables.find(pThis->Param3);;
	pINI->WriteInteger(KeyName, variable->second.Name, variable->second.Value, false);
	//pINI->WriteString(ScenarioClass::Instance()->FileName, variable->second.Name, variable->second.Name);
	pINI->WriteCCFile(pFile);
	pFile->Close();

	return true;
}
bool TActionExt::OutputInteger(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto spcialText = pThis->Text;
	const int maxTokens = 3;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbol(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto Section = result[1];
	const auto KeyName = result[2];
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);
	
	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
		pFile->CreateFileA();

	pINI->WriteInteger(Section, KeyName, pThis->Param3, false);
	//pINI->WriteString(ScenarioClass::Instance()->FileName, variable->second.Name, variable->second.Name);
	pINI->WriteCCFile(pFile);
	pFile->Close();

	return true;
}
bool TActionExt::OutputIntegerRandomly(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto spcialText = pThis->Text;
	const int maxTokens = 3;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbol(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto Section = result[1];
	const auto KeyName = result[2];
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);

	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
		pFile->CreateFileA();
	int output=ScenarioClass::Instance->Random.RandomRanged(pThis->Param3, pThis->Param4);
	pINI->WriteInteger(Section, KeyName, output, false);
	//pINI->WriteString(ScenarioClass::Instance()->FileName, variable->second.Name, variable->second.Name);
	pINI->WriteCCFile(pFile);
	pFile->Close();

	return true;
}
bool TActionExt::OutputIntegerRandomlyTargets(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto spcialText = pThis->Text;
	const int maxTokens = 2;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbol(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto KeyName = result[1];
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);

	if (RulesExt::Global()->AITargetTypesLists.size() > 0
		&& RulesExt::Global()->AITargetTypesLists[pThis->Param5].size() > 0)
	{
		if (pFile->Exists())
			pINI->ReadCCFile(pFile);
		else
			pFile->CreateFileA();
		for (auto item : RulesExt::Global()->AITargetTypesLists[pThis->Param5])
		{
			int output = ScenarioClass::Instance->Random.RandomRanged(pThis->Param3, pThis->Param4);
			pINI->WriteInteger(item->ID, KeyName, output, false);
		}
	}
	else return true;
	//pINI->WriteString(ScenarioClass::Instance()->FileName, variable->second.Name, variable->second.Name);
	pINI->WriteCCFile(pFile);
	pFile->Close();

	return true;
}
bool TActionExt::OutputDouble(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto spcialText = pThis->Text;
	const int maxTokens = 3;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbol(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto Section = result[1];
	const auto KeyName = result[2];
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);

	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
		pFile->CreateFileA();
	double value = std::round(static_cast<double>(pThis->Param3) / pThis->Param4 * 1000.0) / 1000.0;
	pINI->WriteDouble(Section, KeyName, value);
	//pINI->WriteString(ScenarioClass::Instance()->FileName, variable->second.Name, variable->second.Name);
	pINI->WriteCCFile(pFile);
	pFile->Close();

	return true;
}
bool TActionExt::OutputDoubleWithVar(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto& variables = ScenarioExt::Global()->Variables[0];
	const auto spcialText = pThis->Text;
	const int maxTokens = 3;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbol(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto Section = result[1];
	const auto KeyName = result[2];
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);

	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
		pFile->CreateFileA();
	int a = variables.find(pThis->Param3)->second.Value;
	int b= variables.find(pThis->Param4)->second.Value;
	double value = std::round(static_cast<double>(a) / b * 1000.0) / 1000.0;
	pINI->WriteDouble(Section, KeyName, value);
	//pINI->WriteString(ScenarioClass::Instance()->FileName, variable->second.Name, variable->second.Name);
	pINI->WriteCCFile(pFile);
	pFile->Close();

	return true;
}
bool TActionExt::OutputDoubleWithVarCsf(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto& variables = ScenarioExt::Global()->Variables[0];
	const auto spcialText = WCharToUtf8(StringTable::LoadString(pThis->Text));
	const int maxTokens = 3;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbol(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto Section = result[1];
	const auto KeyName = result[2];
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);

	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
		pFile->CreateFileA();
	int a = variables.find(pThis->Param3)->second.Value;
	int b = variables.find(pThis->Param4)->second.Value;
	double value = std::round(static_cast<double>(a) / b * 1000.0) / 1000.0;
	pINI->WriteDouble(Section, KeyName, value);
	//pINI->WriteString(ScenarioClass::Instance()->FileName, variable->second.Name, variable->second.Name);
	pINI->WriteCCFile(pFile);
	pFile->Close();

	return true;
}
bool TActionExt::EditOuterIntegers(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto spcialText = pThis->Text;
	const int maxTokens = 2;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbol(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto KeyName = result[1];
	auto& variables = ScenarioExt::Global()->Variables[0];
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);
	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
	{
		pFile->CreateFileA();
	}

	auto itr = variables.find(pThis->Param3);
	int nCurrentValue = 0;
	if (itr != variables.end())
	{
		nCurrentValue = itr->second.Value;
	}	
	if (RulesExt::Global()->AITargetTypesLists.size() > 0
		&& RulesExt::Global()->AITargetTypesLists[pThis->Param5].size() > 0)
	{
		for (auto item : RulesExt::Global()->AITargetTypesLists[pThis->Param5])
		{
			int value = pINI->ReadInteger(item->ID, KeyName, 0);
			switch (pThis->Param4)
			{				// variable being found
		   	case 0: { value = nCurrentValue; break; }
			case 1: { value += nCurrentValue; break; }
			case 2: { value -= nCurrentValue; break; }
			case 3: { value *= nCurrentValue; break; }
			case 4: { value /= nCurrentValue; break; }
			case 5: { value %= nCurrentValue; break; }
			case 6: { value <<= nCurrentValue; break; }
			case 7: { value >>= nCurrentValue; break; }
			case 8: { value = ~value; break; }
			case 9: { value ^= nCurrentValue; break; }
			case 10: { value |= nCurrentValue; break; }
			case 11: { value &= nCurrentValue; break; }
			default:
				return true;
			}

			pINI->WriteInteger(item->ID, KeyName, value, false);
		}
	}
	else return true;
	pINI->WriteCCFile(pFile);
	pFile->Close();

	return true;
}
bool TActionExt::EditOuterInteger(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto spcialText = pThis->Text;
	const int maxTokens = 3;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbol(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto SectionName = result[1];
	const auto KeyName = result[2];
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);
	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
	{
		pFile->CreateFileA();
	}
	int value = pINI->ReadInteger(SectionName, KeyName, 0);
	int nCurrentValue = pThis->Param4;
	switch (pThis->Param3)
	{				// variable being found
		case 0: { value = nCurrentValue; break; }
		case 1: { value += nCurrentValue; break; }
		case 2: { value -= nCurrentValue; break; }
		case 3: { value *= nCurrentValue; break; }
		case 4: { value /= nCurrentValue; break; }
		case 5: { value %= nCurrentValue; break; }
		case 6: { value <<= nCurrentValue; break; }
		case 7: { value >>= nCurrentValue; break; }
		case 8: { value = ~value; break; }
		case 9: { value ^= nCurrentValue; break; }
		case 10: { value |= nCurrentValue; break; }
		case 11: { value &= nCurrentValue; break; }
		default:
			return true;
	}

	pINI->WriteInteger(SectionName, KeyName, value, false);
	pINI->WriteCCFile(pFile);
	pFile->Close();

	return true;
}
bool TActionExt::EditOuterDoubles(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto spcialText = WCharToUtf8(StringTable::LoadString(pThis->Text));
	const int maxTokens = 4;
	char result[maxTokens][120];
	char delimiter = '@';
	splitByAtSymbolLarge(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto SectionName = result[1];
	const auto KeyName = result[2];
	const auto number = result[3];
	double nCurrentValue = std::stod(number);
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);
	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
	{
		pFile->CreateFileA();
	}
	double value = pINI->ReadDouble(SectionName, KeyName, 0);

    switch (pThis->Param3)
	{				// variable being found
		case 0: { value = nCurrentValue; break; }
		case 1: { value += nCurrentValue; break; }
		case 2: { value -= nCurrentValue; break; }
		case 3: { value *= nCurrentValue; break; }
		case 4: { value /= nCurrentValue; break; }
		default:
			return true;
	}
    pINI->WriteDouble(SectionName, KeyName, value);
	pINI->WriteCCFile(pFile);
	pFile->Close();

	return true;
}
/*
bool Read(INIClass* pINI, const char* pSection, const char* pKey, const char* pDefault = "")
{
	if (pINI->ReadString(pSection, pKey, pDefault, Phobos::readBuffer, FixedString<Capacity>::Size))
	{
		if (!INIClass::IsBlank(Phobos::readBuffer))
		{
			*this = Phobos::readBuffer;
		}
		else
		{
			*this = nullptr;
		}
	}
	return Phobos::readBuffer[0] != 0;
}*/
bool TActionExt::EditOuterStrings(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto spcialText = WCharToUtf8(StringTable::LoadString(pThis->Text));
	const int maxTokens = 4;
	char result[maxTokens][120];
	char delimiter = '@';
	splitByAtSymbolLarge(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto SectionName = result[1];
	const auto KeyName = result[2];
	const auto ustring = result[3];
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);
	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
	{
		pFile->CreateFileA();
	}
	char* str = new char[200];
	std::strcpy(str, "");
	if (pINI->ReadString(SectionName, KeyName, "", Phobos::readBuffer, FixedString<0x200>::Size))
	{
		if (!INIClass::IsBlank(Phobos::readBuffer))
		{
			str= Phobos::readBuffer;
		}
		else
		{
			std::strcpy(str, "");
		}
	}
	if (pThis->Param3==0)
	{
		std::strcat(str, ustring);
		pINI->WriteString(SectionName, KeyName, str);
	}
	else if(pThis->Param3 == 1)
	{
		std::strcat(ustring, str);
		pINI->WriteString(SectionName, KeyName, ustring);
	}
	else
	{
		pINI->WriteString(SectionName, KeyName, ustring);
	}
	pINI->WriteCCFile(pFile);
	pFile->Close();
	delete[] str;
	delete[] ustring;
	return true;

}
bool TActionExt::ReadVariablesFromFile(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto spcialText = pThis->Text;
	const int maxTokens = 3;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbol(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto KeyName= result[1];
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
	int value = pINI->ReadInteger(KeyName, VariableName, 0);
	auto& variables = ScenarioExt::Global()->Variables[0];
	int i = 0;
	for (i = 0; i < variables.size(); i++)
	{
		auto variable = variables.find(i);
		if (strcmp(variable->second.Name, VariableName) == 0)
		{
			variable->second.Value = value;
			TagClass::NotifyLocalChanged(i);
			return true;
		}

	}
	return true;
}
bool TActionExt::OutputStringToFile(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto spcialText = pThis->Text;
	const int maxTokens = 4;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbol(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto KeyName = result[1];
	const auto csfName = result[2];
	const auto csfName2 = result[3];
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);

	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
		pFile->CreateFileA();
	const auto vName = WCharToUtf8(StringTable::LoadString(csfName));
	const auto vName2 = WCharToUtf8(StringTable::LoadString(csfName2));
	pINI->WriteString(KeyName, vName, vName2);
	pINI->WriteCCFile(pFile);
	pFile->Close();

	return true;
}
bool TActionExt::PlayAudioAtRandomWP(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	std::vector<int> waypoints;
	waypoints.reserve(ScenarioExt::Global()->Waypoints.size());

	auto const pScen = ScenarioClass::Instance();

	for (auto pair : ScenarioExt::Global()->Waypoints)
		if (pScen->IsDefinedWaypoint(pair.first))
			waypoints.push_back(pair.first);

	if (waypoints.size() > 0)
	{
		auto const index = pScen->Random.RandomRanged(0, waypoints.size() - 1);
		auto const luckyWP = waypoints[index];
		auto const cell = pScen->GetWaypointCoords(luckyWP);
		auto const coords = CellClass::Cell2Coord(cell);
		VocClass::PlayIndexAtPos(pThis->Value, coords);
	}

	return true;
}

bool TActionExt::SaveGame(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (SessionClass::IsSingleplayer())
	{
		auto PrintMessage = [](const wchar_t* pMessage)
		{
			MessageListClass::Instance->PrintMessage(
				pMessage,
				RulesClass::Instance->MessageDelay,
				HouseClass::CurrentPlayer->ColorSchemeIndex,
				true
			);
		};

		char fName[0x80];

		SYSTEMTIME time;
		GetLocalTime(&time);

		_snprintf_s(fName, 0x7F, "Map.%04u%02u%02u-%02u%02u%02u-%05u.sav",
			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

		PrintMessage(StringTable::LoadString(GameStrings::TXT_SAVING_GAME));

		wchar_t fDescription[0x80] = { 0 };
		wcscpy_s(fDescription, ScenarioClass::Instance->UINameLoaded);
		wcscat_s(fDescription, L" - ");
		wcscat_s(fDescription, StringTable::LoadString(pThis->Text));

		if (ScenarioClass::Instance->SaveGame(fName, fDescription))
			PrintMessage(StringTable::LoadString(GameStrings::TXT_GAME_WAS_SAVED));
		else
			PrintMessage(StringTable::LoadString(GameStrings::TXT_ERROR_SAVING_GAME));
	}

	return true;
}

bool TActionExt::EditVariable(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	// Variable Index
	// holds by pThis->Value

	// Operations:
	// 0 : set value - operator=
	// 1 : add value - operator+
	// 2 : minus value - operator-
	// 3 : multiply value - operator*
	// 4 : divide value - operator/
	// 5 : mod value - operator%
	// 6 : <<
	// 7 : >>
	// 8 : ~ (no second param being used)
	// 9 : ^
	// 10 : |
	// 11 : &
	// holds by pThis->Param3

	// Params:
	// The second value
	// holds by pThis->Param4

	// Global Variable or Local
	// 0 for local and 1 for global
	// holds by pThis->Param5

	// uses !pThis->Param5 to ensure Param5 is 0 or 1
	auto& variables = ScenarioExt::Global()->Variables[pThis->Param5 != 0];
	auto itr = variables.find(pThis->Value);
	if (itr != variables.end())
	{
		auto& nCurrentValue = itr->second.Value;
		// variable being found
		switch (pThis->Param3)
		{
		case 0: { nCurrentValue = pThis->Param4; break; }
		case 1: { nCurrentValue += pThis->Param4; break; }
		case 2: { nCurrentValue -= pThis->Param4; break; }
		case 3: { nCurrentValue *= pThis->Param4; break; }
		case 4: { nCurrentValue /= pThis->Param4; break; }
		case 5: { nCurrentValue %= pThis->Param4; break; }
		case 6: { nCurrentValue <<= pThis->Param4; break; }
		case 7: { nCurrentValue >>= pThis->Param4; break; }
		case 8: { nCurrentValue = ~nCurrentValue; break; }
		case 9: { nCurrentValue ^= pThis->Param4; break; }
		case 10: { nCurrentValue |= pThis->Param4; break; }
		case 11: { nCurrentValue &= pThis->Param4; break; }
		default:
			return true;
		}

		if (!pThis->Param5)
			TagClass::NotifyLocalChanged(pThis->Value);
		else
			TagClass::NotifyGlobalChanged(pThis->Value);
	}
	return true;
}

bool TActionExt::GenerateRandomNumber(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto& variables = ScenarioExt::Global()->Variables[pThis->Param5 != 0];
	auto itr = variables.find(pThis->Value);
	if (itr != variables.end())
	{
		itr->second.Value = ScenarioClass::Instance->Random.RandomRanged(pThis->Param3, pThis->Param4);
		if (!pThis->Param5)
			TagClass::NotifyLocalChanged(pThis->Value);
		else
			TagClass::NotifyGlobalChanged(pThis->Value);
	}

	return true;
}

bool TActionExt::PrintVariableValue(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto& variables = ScenarioExt::Global()->Variables[pThis->Param3 != 0];
	auto itr = variables.find(pThis->Value);
	if (itr != variables.end())
	{
		CRT::swprintf(Phobos::wideBuffer, L"%d", itr->second.Value);
		MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
	}

	return true;
}

bool TActionExt::BinaryOperation(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto& variables1 = ScenarioExt::Global()->Variables[pThis->Param5 != 0];
	auto itr1 = variables1.find(pThis->Value);
	auto& variables2 = ScenarioExt::Global()->Variables[pThis->Param6 != 0];
	auto itr2 = variables2.find(pThis->Param4);

	if (itr1 != variables1.end() && itr2 != variables2.end())
	{
		auto& nCurrentValue = itr1->second.Value;
		auto& nOptValue = itr2->second.Value;
		switch (pThis->Param3)
		{
		case 0: { nCurrentValue = nOptValue; break; }
		case 1: { nCurrentValue += nOptValue; break; }
		case 2: { nCurrentValue -= nOptValue; break; }
		case 3: { nCurrentValue *= nOptValue; break; }
		case 4: { nCurrentValue /= nOptValue; break; }
		case 5: { nCurrentValue %= nOptValue; break; }
		case 6: { nCurrentValue <<= nOptValue; break; }
		case 7: { nCurrentValue >>= nOptValue; break; }
		case 8: { nCurrentValue = nOptValue; break; }
		case 9: { nCurrentValue ^= nOptValue; break; }
		case 10: { nCurrentValue |= nOptValue; break; }
		case 11: { nCurrentValue &= nOptValue; break; }
		default:
			return true;
		}

		if (!pThis->Param5)
			TagClass::NotifyLocalChanged(pThis->Value);
		else
			TagClass::NotifyGlobalChanged(pThis->Value);
	}
	return true;
}

//AI目标类别（需定义AITargetTypes）、超武ID、使用所属方、目标所属方
bool TActionExt::RunSuperWeaponAtRandomUnit(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pThis)
		return true;
	std::vector<CellStruct> c;
	int j = 0;

	if (RulesExt::Global()->AITargetTypesLists.size() > 0
		&& RulesExt::Global()->AITargetTypesLists[pThis->Value].size() > 0)
	{
		HouseClass* targetHouse = HouseClass::Array->GetItem(pThis->Param5);
		for (auto const pTechno : *TechnoClass::Array())
		{
			if (pTechno->Owner == targetHouse)
			{
				for (auto item : RulesExt::Global()->AITargetTypesLists[pThis->Value])
				{
					if (pTechno->GetTechnoType() == item&&!pTechno->InLimbo&&pTechno->IsOnMap&&pTechno->IsAlive)
					{
						if (pThis->Param6 == 0)
						{
							CellStruct cell = { pTechno->GetMapCoords().X ,pTechno->GetMapCoords().Y };
							/*auto pSuper = pHouse->Supers.GetItem(pThis->Param3);
							const auto pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type);
							if (!pSWExt->HasInhibitor(pHouse, cell))*/
							c.push_back(cell);
							j++;
						}
						else
						{
							TActionExt::RunSuperWeaponAt(pThis, pTechno->GetMapCoords().X, pTechno->GetMapCoords().Y);
							return true;
						}

					}
				}
			}
		}
	}
	if (j == 0) return true;
	int i = ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(c.size())-1);
	TActionExt::RunSuperWeaponAt(pThis, c[i].X, c[i].Y);
	return true;
}
int findIndexInD(const std::vector<TechnoTypeClass*>& d, TechnoTypeClass* type)
{
	for (size_t i = 0; i < d.size(); ++i)
	{
		if (d[i] == type)
		{
			return static_cast<int>(i);
		}
	}
	// 如果没找到，返回-1或抛出异常，取决于你的需求  
	return -1;
}

// 自定义比较函数，用于排序  
bool compareTechnoClasses(TechnoClass* a, TechnoClass* b, const std::vector<TechnoTypeClass*>& d)
{
	int indexA = findIndexInD(d, a->GetTechnoType());
	int indexB = findIndexInD(d, b->GetTechnoType());
	return indexA < indexB;
}
std::vector<TechnoClass*> findTechnosWithSameType(const std::vector<TechnoClass*>& c, TechnoTypeClass* type)
{
	std::vector<TechnoClass*> sameTypeTechnos;
	for (TechnoClass* techno : c)
	{
		if (techno->GetTechnoType() == type)
		{
			sameTypeTechnos.push_back(techno);
		}
	}
	return sameTypeTechnos;
}
//AI目标类别（需定义AITargetTypes）、超武ID、使用所属方、目标所属方、第二优先级AI目标类别
bool TActionExt::RunSuperWeaponAtRandomUnit2(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pThis)
		return true;
	std::vector<TechnoClass*> c;
	std::vector<TechnoTypeClass*> d= RulesExt::Global()->AITargetTypesLists[pThis->Value];
	int j = 0;
	if (RulesExt::Global()->AITargetTypesLists.size() > 0
		&& RulesExt::Global()->AITargetTypesLists[pThis->Value].size() > 0)
	{
		HouseClass* targetHouse = HouseClass::Array->GetItem(pThis->Param5);
		for (auto const pTechno : *TechnoClass::Array())
		{
			if (pTechno->Owner == targetHouse)
			{
				for (auto item : RulesExt::Global()->AITargetTypesLists[pThis->Value])
				{
					if (pTechno->GetTechnoType() == item && !pTechno->InLimbo && pTechno->IsOnMap && pTechno->IsAlive)
					{
						c.push_back(pTechno);
						j++;
					}
				}
			}
		}
	}
	if (j > 0)
	{
		//std::sort(c.begin(), c.end(), std::bind(compareTechnoClasses, std::placeholders::_1, std::placeholders::_2, std::cref(d)));
		std::vector<TechnoClass*>e = findTechnosWithSameType(c,c[0]->GetTechnoType());
		int i = ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(e.size()) - 1);
		CellStruct cell = { e[i]->GetMapCoords().X ,e[i]->GetMapCoords().Y };
		TActionExt::RunSuperWeaponAt(pThis, cell.X, cell.Y);

	}

	
	return true;
}
bool TActionExt::RunSuperWeaponAtLocation(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pThis)
		return true;

	TActionExt::RunSuperWeaponAt(pThis, pThis->Param5, pThis->Param6);

	return true;
}

bool TActionExt::RunSuperWeaponAtWaypoint(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pThis)
		return true;

	auto& waypoints = ScenarioExt::Global()->Waypoints;
	int nWaypoint = pThis->Param5;

	// Check if is a valid Waypoint
	if (nWaypoint >= 0 && waypoints.find(nWaypoint) != waypoints.end() && waypoints[nWaypoint].X && waypoints[nWaypoint].Y)
	{
		auto const selectedWP = waypoints[nWaypoint];
		TActionExt::RunSuperWeaponAt(pThis, selectedWP.X, selectedWP.Y);
	}

	return true;
}

bool TActionExt::RunSuperWeaponAt(TActionClass* pThis, int X, int Y)
{
	if (SuperWeaponTypeClass::Array->Count > 0)
	{
		int swIdx = pThis->Param3;
		int houseIdx = -1;
		std::vector<int> housesListIdx;
		CellStruct targetLocation = { (short)X, (short)Y };
		do
		{
			if (X < 0)
				targetLocation.X = (short)ScenarioClass::Instance->Random.RandomRanged(0, MapClass::Instance->MapCoordBounds.Right);

			if (Y < 0)
				targetLocation.Y = (short)ScenarioClass::Instance->Random.RandomRanged(0, MapClass::Instance->MapCoordBounds.Bottom);
		}
		while (!MapClass::Instance->IsWithinUsableArea(targetLocation, false));

		// Only valid House indexes
		if ((pThis->Param4 >= HouseClass::Array->Count
			&& pThis->Param4 < HouseClass::PlayerAtA)
			|| pThis->Param4 > (HouseClass::PlayerAtA + HouseClass::Array->Count - 3))
		{
			return true;
		}

		switch (pThis->Param4)
		{
		case HouseClass::PlayerAtA:
			houseIdx = 0;
			break;

		case HouseClass::PlayerAtB:
			houseIdx = 1;
			break;

		case HouseClass::PlayerAtC:
			houseIdx = 2;
			break;

		case HouseClass::PlayerAtD:
			houseIdx = 3;
			break;

		case HouseClass::PlayerAtE:
			houseIdx = 4;
			break;

		case HouseClass::PlayerAtF:
			houseIdx = 5;
			break;

		case HouseClass::PlayerAtG:
			houseIdx = 6;
			break;

		case HouseClass::PlayerAtH:
			houseIdx = 7;
			break;

		case -1:
			// Random non-neutral
			for (auto pHouse : *HouseClass::Array)
			{
				if (!pHouse->Defeated
					&& !pHouse->IsObserver()
					&& !pHouse->Type->MultiplayPassive)
				{
					housesListIdx.push_back(pHouse->ArrayIndex);
				}
			}

			if (housesListIdx.size() > 0)
				houseIdx = housesListIdx.at(ScenarioClass::Instance->Random.RandomRanged(0, housesListIdx.size() - 1));
			else
				return true;

			break;

		case -2:
			// Find first Neutral
			for (auto pHouseNeutral : *HouseClass::Array)
			{
				if (pHouseNeutral->IsNeutral())
				{
					houseIdx = pHouseNeutral->ArrayIndex;
					break;
				}
			}

			if (houseIdx < 0)
				return true;

			break;

		case -3:
			// Random Human Player
			for (auto pHouse : *HouseClass::Array)
			{
				if (pHouse->IsControlledByHuman()
					&& !pHouse->Defeated
					&& !pHouse->IsObserver())
				{
					housesListIdx.push_back(pHouse->ArrayIndex);
				}
			}

			if (housesListIdx.size() > 0)
				houseIdx = housesListIdx.at(ScenarioClass::Instance->Random.RandomRanged(0, housesListIdx.size() - 1));
			else
				return true;

			break;

		default:
			if (pThis->Param4 >= 0)
				houseIdx = pThis->Param4;
			else
				return true;

			break;
		}

		if (HouseClass* pHouse = HouseClass::Array->GetItem(houseIdx))
		{
			if (auto const pSuper = pHouse->Supers.GetItem(swIdx))
			{
				int oldstart = pSuper->RechargeTimer.StartTime;
				int oldleft = pSuper->RechargeTimer.TimeLeft;
				pSuper->SetReadiness(true);
				pSuper->Launch(targetLocation, false);
				pSuper->Reset();
				pSuper->RechargeTimer.StartTime = oldstart;
				pSuper->RechargeTimer.TimeLeft = oldleft;
			}
		}
	}

	return true;
}
bool TActionExt::UpdateNextScenario(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	ScenarioExt::Global()->NextMission = pThis->Text;
	return true;
}
bool TActionExt::UpdateMoney(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto& var = ScenarioExt::Global()->Variables[0];
	auto variable = var.find(pThis->Value);

	if (variable != var.end())
	{
		HouseClass* foundHouse = HouseClass::Array->GetItem(pThis->Param3);
		if (!foundHouse)
			return false;
		foundHouse->TransactMoney(variable->second.Value);
	}
	return true;
}
bool TActionExt::WriteMessage(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto spcialText = pThis->Text;
	const int maxTokens = 2;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbol(spcialText, delimiter, result, maxTokens);
	const auto csfLabel = result[0];
	const auto ColorName = result[1];
	int frames = pThis->Param3;
	if (frames <= 0)
		frames = 150;
	wchar_t* pMessage = csfConvert(StringTable::LoadString(csfLabel));

	MessageListClass::Instance->PrintMessage(pMessage, frames, ColorScheme::FindIndex(ColorName), false);
	delete[] pMessage;
	return true;
}
bool TActionExt::UpdateMaxMessageCount(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	MessageListClass::Instance->MaxMessageCount = pThis->Value;
	return true;
}
bool TActionExt::UpdateMessageBuffer(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	wchar_t* pMessage = csfConvert(StringTable::LoadString(pThis->Text));	
	size_t length = wcslen(pMessage);
	int len = static_cast<int>(length);
	std::wmemset(MessageListClass::Instance->MessageBuffers[pThis->Param3], 0, 162);
	for (int j = 0; j < len; j++)
		MessageListClass::Instance->MessageBuffers[pThis->Param3][j] = pMessage[j];
	return true;
}
bool TActionExt::ToggleMCVRedeploy(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	GameModeOptionsClass::Instance->MCVRedeploy = pThis->Param3 != 0;
	return true;
}

bool TActionExt::BindTeamMemberToTag(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto spcialText = pThis->Text;
	const int maxTokens = 2;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbol(spcialText, delimiter, result, maxTokens);
	const auto TeamName = result[0];
	const auto TagName = result[1];

	TagClass* tag;
	for (auto const pTag : *TagClass::Array())
	{
		if (!strcmp(pTag->Type->get_ID(), TagName))
		{
			tag = pTag;
			break;
		}
	}
	for (auto const pTechno : *TechnoClass::Array())
	{
		if (pTechno->WhatAmI() != AbstractType::BuildingType)
		{
			if (FootClass* pFoot = abstract_cast<FootClass*>(pTechno))
			{
				if (pFoot->BelongsToATeam() && pFoot->Team&& strcmp(pFoot->Team->Type->get_ID(),TeamName)==0)
				{
					for (auto pUnit = pFoot->Team->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
					{
						pUnit->AttachTrigger(tag);
					}
				}
			}
		}
	}
	return true;
}
bool TActionExt::BindTechnologyToTag(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto spcialText = pThis->Text;
	const int maxTokens = 2;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbol(spcialText, delimiter, result, maxTokens);
	const auto TechName = result[0];
	const auto TagName = result[1];
	wchar_t* pMessage = ConvertWchar(TagName);
	MessageListClass::Instance->PrintMessage(pMessage);
	HouseClass* foundHouse = HouseClass::Array->GetItem(pThis->Param3);
	pMessage = ConvertWchar(foundHouse->get_ID());
	MessageListClass::Instance->PrintMessage(pMessage);
	if (!foundHouse)
		return false;
	TagClass* tag;
	for (auto const pTag : *TagClass::Array())
	{
		if (!strcmp(pTag->Type->get_ID(), TagName))
		{
			tag = pTag;
			break;
		}
	}
	for (auto const pTechno : *TechnoClass::Array())
	{		
		if (pTechno->Owner==foundHouse&&strcmp(TechName, pTechno->get_ID()) == 0)
		{
			pTechno->AttachTrigger(tag);
		}
	}
	return true;
}

void CreateOrReplaceBanner(TActionClass* pTAction, bool isGlobal)
{
	BannerTypeClass* pBannerType = BannerTypeClass::Array[pTAction->Param3].get();

	auto& banners = BannerClass::Array;
    const auto it = std::find_if(banners.cbegin(), banners.cend(),
		[pTAction](const std::unique_ptr<BannerClass>& pBanner)
		{
			return pBanner->ID == pTAction->Value;
		});

	if (it != banners.cend())
	{
		const auto& pBanner = *it;
		pBanner->Type = pBannerType;
		pBanner->Position = CoordStruct(pTAction->Param4, pTAction->Param5, 0);
		pBanner->Variable = pTAction->Param6;
		pBanner->IsGlobalVariable = isGlobal;
	}
	else
	{
		BannerClass::Array.emplace_back
		(
			std::make_unique<BannerClass>
			(
				pBannerType,
				pTAction->Value,
				CoordStruct(pTAction->Param4, pTAction->Param5, 0),
				pTAction->Param6,
				isGlobal
				)
		); 
	}
}


bool TActionExt::CreateBannerLocal(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	CreateOrReplaceBanner(pThis, false);
	return true;
}

bool TActionExt::DeleteBanner(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto it = std::find_if(BannerClass::Array.cbegin(), BannerClass::Array.cend(),
		[pThis](const std::unique_ptr<BannerClass>& pBanner)
		{
			return pBanner->ID == pThis->Value;
		});

	if (it != BannerClass::Array.cend())
		BannerClass::Array.erase(it);

	return true;
}

bool TActionExt::EnableTriggerWithMark(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{

	for (auto const trigger : *TriggerClass::Array())
	{
		std::string t = pThis->Text;
		if (Contains("[" + t + "]", trigger->Type->Name)&&!trigger->Destroyed)
		{
			trigger->Enable();
		}
	}
	return true;
}
bool TActionExt::DisableTriggerrWithMark(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	for (auto const trigger : *TriggerClass::Array())
	{
		std::string t = pThis->Text;
		if (Contains("[" + t + "]", trigger->Type->Name) && !trigger->Destroyed)
		{
			trigger->Disable();
		}
	}
	return true;
}
bool TActionExt::EnableTriggerWithMarkRandomly(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	std::vector<TriggerClass*> triggers;
	for (auto const trigger : *TriggerClass::Array())
	{
		std::string t = pThis->Text;
		if (Contains("[" + t + "]", trigger->Type->Name) && !trigger->Enabled && !trigger->Destroyed)
		{
			triggers.push_back(trigger);
		}
	}
	int i = ScenarioClass::Instance->Random.RandomRanged(0, triggers.size() - 1);
	triggers[i]->Enable();
	return true;
}
bool TActionExt::CreateTeamWithGroupRandomly(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	std::vector<TeamTypeClass*> teamtypes;
	for (auto pTeamType : *TeamTypeClass::Array())
	{
		if (pThis->Value == pTeamType->Group)
		{
			teamtypes.push_back(pTeamType);
		}
	}
	int i = ScenarioClass::Instance->Random.RandomRanged(0, teamtypes.size() - 1);
	pThis->TeamType = teamtypes[i];
	TActionExt::CreateTeam(pThis, pHouse, pObject, pTrigger, location);
	return true;
}
bool TActionExt::CreateTeamWithGroup(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	for (auto pTeamType : *TeamTypeClass::Array())
	{
		if (pThis->Value == pTeamType->Group)
		{
			pThis->TeamType = pTeamType;
			TActionExt::CreateTeam(pThis, pHouse, pObject, pTrigger, location);
		}
	}
	return true;
}
bool TActionExt::CreateTeam(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	TaskForceEntryStruct* tStruct = pThis->TeamType->TaskForce->Entries;
	HouseClass* house = pThis->TeamType->Owner;
	auto& waypoints = ScenarioExt::Global()->Waypoints;
	int nWaypoint = pThis->TeamType->Waypoint;
	CellStruct selectedWP;
	int level = pThis->TeamType->VeteranLevel;
	bool full = pThis->TeamType->Full;
	bool dropped = pThis->TeamType->Droppod;
	std::vector<TechnoTypeClass*> transports;
	std::vector<FootClass*> foots;
	std::vector<FootClass*> foots2;
	int maxsize = 0;
	TeamClass* pTeam = GameCreate<TeamClass>(pThis->TeamType, house, 0);
	 if (nWaypoint >= 0 && waypoints.find(nWaypoint) != waypoints.end() && waypoints[nWaypoint].X && waypoints[nWaypoint].Y)
	{
		selectedWP = waypoints[nWaypoint];
	}
	else
	{
		return true;
	};
	 if (full)
	{
		// Collect available transports

		for (int i = 0; i < 6; ++i)
		{
			TaskForceEntryStruct currentEntry = tStruct[i];
			FootClass* pFoot;
			if (currentEntry.Amount > 0)
			{
				auto pType = currentEntry.Type;
				if (pType->Passengers > 0)
				{
					transports.emplace_back(pType);
					if (pType->Passengers > maxsize) maxsize = pType->Passengers;
				}
			}
		}
	}
	 if (dropped)
	 {
		 for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		 {
			 pTeam->LiberateMember(pUnit);
		 }
		 auto pPlaneType = AircraftTypeClass::Array->GetItem(AircraftTypeClass::FindIndex("PDPLANE"));
		 auto pTarget = MapClass::Instance->TryGetCellAt(selectedWP);
		 ++Unsorted::IKnowWhatImDoing;
		 auto const pPlane = abstract_cast<AircraftClass*>(pPlaneType->CreateObject(pHouse));
		 --Unsorted::IKnowWhatImDoing;
		 pPlane->Spawned = true;
		 //Get edge (direction for plane to come from)
		 auto edge = pHouse->StartingEdge;
		 if (edge < Edge::North || edge > Edge::West)
		 {
			 edge = pHouse->Edge;
			 if (edge < Edge::North || edge > Edge::West)
			 {
				 edge = Edge::North;
			 }
		 }
		 int startf = pTeam->Type->TransportWaypoint;
		 CellStruct  spawn_cell;
		 if (startf >= 0)
		 {
			 spawn_cell = waypoints[startf];
		 }
		 // seems to retrieve a random cell struct at a given edge
		 else
		 {
			 spawn_cell = MapClass::Instance->PickCellOnEdge(
				 edge, CellStruct::Empty, CellStruct::Empty, SpeedType::Winged, true,
				 MovementZone::Normal);
		 }
		 pPlane->QueueMission(Mission::ParadropApproach, false);

		 if (pTarget)
		 {
			 pPlane->SetTarget(pTarget);
		 }

		 auto const spawn_crd = CellClass::Cell2Coord(spawn_cell);

		 ++Unsorted::IKnowWhatImDoing;
		 auto const bSpawned = pPlane->Unlimbo(spawn_crd, DirType::NorthEast);
		 --Unsorted::IKnowWhatImDoing;

		 if (!bSpawned)
		 {
			 GameDelete(pPlane);
			 return true;
		 }
		 for (int i = 0; i < 6; ++i)
		 {
			 TaskForceEntryStruct currentEntry = tStruct[i];
			 if (currentEntry.Amount > 0)
			 {
				 auto pTechno = currentEntry.Type;
				 if (pTechno->WhatAmI() != AbstractType::BuildingType)
				 {		
					 for (int k = 0; k < currentEntry.Amount; ++k)
					 {
						 ++Unsorted::IKnowWhatImDoing;
						 auto const pNew = pTechno->CreateObject(house);
						 auto pFoot = static_cast<FootClass*>(pNew);
						 pTeam->AddMember(pFoot, true);
						 --Unsorted::IKnowWhatImDoing;

						 if (level == 2)
						 {
							 pFoot->Veterancy.SetVeteran();
						 }
						 else if (level >= 3)
						 {
							 pFoot->Veterancy.SetElite();
						 }
						 pFoot->Limbo();
						 pPlane->Passengers.AddPassenger(static_cast<FootClass*>(pNew));
					 }
				 }
				 else
				 {
					 return true;
				 }
			 }
		 }
		 pPlane->HasPassengers = true;
		 pPlane->NextMission();
	 }
	 else if (transports.size() > 0)
	 {
		for (int i = 0; i < 6; ++i)
		{
			TaskForceEntryStruct currentEntry = tStruct[i];
			FootClass* pFoot;
			if (currentEntry.Amount > 0)
			{
				auto pType = currentEntry.Type;
				if (pType->WhatAmI() == AbstractType::BuildingType) return true;
				if (containsPointer(transports, pType)&&pType->Passengers==maxsize)
				{
					for (int j = 0; j < currentEntry.Amount; j++)
					{
						pFoot = static_cast<FootClass*>(pType->CreateObject(house));
						if (level == 2)
						{
							pFoot->Veterancy.SetVeteran();
						}
						else if (level >= 3)
						{
							pFoot->Veterancy.SetElite();
						}
						foots.emplace_back(pFoot);
					}
				}
			}
		}
		for (int i = 0; i < 6; ++i)
		{
			TaskForceEntryStruct currentEntry = tStruct[i];
			FootClass* pFoot;
			if (currentEntry.Amount > 0)
			{
				auto pType = currentEntry.Type;
				if (pType->WhatAmI() == AbstractType::BuildingType) return true;
				if (pType->Passengers<maxsize)
				{
					for (int j = 0; j < currentEntry.Amount; j++)
					{
						pFoot = static_cast<FootClass*>(pType->CreateObject(house));
						if (level == 2)
						{
							pFoot->Veterancy.SetVeteran();
						}
						else if (level >= 3)
						{
							pFoot->Veterancy.SetElite();
						}
						foots2.emplace_back(pFoot);
					}
				}
			}
		}
		std::vector<FootClass*>toDelete;
		auto pCell = MapClass::Instance->TryGetCellAt(selectedWP);
		const CoordStruct locat = pCell->GetCoords();
		for (auto pFoot : foots)
		{
			auto techtype = pFoot->GetTechnoType();
			int size = techtype->Passengers;
			int limit = techtype->SizeLimit;
			int sizeleft = size;

			FootClass* pGunner = nullptr;
			bool topped = pFoot->GetTechnoType()->OpenTopped;
			for (auto pUnit : foots2)
			{
				auto utype = pUnit->GetTechnoType();
				if (sizeleft > 0 && utype->Size <= limit && utype->Size <= sizeleft && !containsPointer(toDelete, pUnit))
				{
					pGunner = pUnit;
					if (topped)
					{
						pFoot->EnteredOpenTopped(pUnit);
					}
					pUnit->Transporter = pFoot;
					pFoot->AddPassenger(pUnit);
					pTeam->AddMember(pUnit, true);
					toDelete.push_back(pUnit);
					sizeleft -= utype->Size;
				}
			}
			if (pFoot->GetTechnoType()->Gunner && pGunner)
				pFoot->ReceiveGunner(pGunner);
			bool success;
			if (!pCell->GetBuilding())
			{
				++Unsorted::IKnowWhatImDoing;
				success = pFoot->Unlimbo(locat, DirType::NorthEast);
				--Unsorted::IKnowWhatImDoing;
				pTeam->AddMember(pFoot, true);
			}
			else
			{
				success = pFoot->Unlimbo(locat, DirType::NorthEast);
				pTeam->AddMember(pFoot, true);
			}
		}
		bool suc;
		for (auto pUnit : foots2)
		{
			if (!containsPointer(toDelete, pUnit))
			{
				++Unsorted::IKnowWhatImDoing;
				suc= pUnit->Unlimbo(locat, DirType::NorthEast);
				pTeam->AddMember(pUnit, true);
				--Unsorted::IKnowWhatImDoing;
			}
		}
		pTeam->IsForcedActive = true;
		pTeam->CurrentScript = GameCreate<ScriptClass>(pThis->TeamType->ScriptType);
		pTeam->StepCompleted = true;

	}
//	TeamClass* pTeam = pThis->TeamType->CreateTeam(house);
	// Check if is a valid Waypoint
	else
	{
		for (int i = 0; i < 6; ++i)
		{
			TaskForceEntryStruct currentEntry = tStruct[i];
			if (currentEntry.Amount > 0)
			{
				auto pTechno = currentEntry.Type;
				FootClass* pFoot;
				if (pTechno->WhatAmI() != AbstractType::BuildingType)
				{
					for (int j = 0; j < currentEntry.Amount; j++)
					{
						pFoot = static_cast<FootClass*>(pTechno->CreateObject(house));
						if (level == 2)
						{
							pFoot->Veterancy.SetVeteran();
						}
						else if (level >= 3)
						{
							pFoot->Veterancy.SetElite();
						}
						bool success;
						auto pCell = MapClass::Instance->TryGetCellAt(selectedWP);
						const CoordStruct locat = pCell->GetCoords();
						if (!pCell->GetBuilding())
						{
							++Unsorted::IKnowWhatImDoing;
							success = pFoot->Unlimbo(locat, DirType::NorthEast);
							--Unsorted::IKnowWhatImDoing;
							pTeam->AddMember(pFoot, true);
						}
						else
						{
							success = pFoot->Unlimbo(locat, DirType::NorthEast);
							pTeam->AddMember(pFoot, true);
						}

					}
				}
				else
				{
					return true;
				}
			}
		}
		pTeam->CurrentScript = GameCreate<ScriptClass>(pThis->TeamType->ScriptType);
		pTeam->StepCompleted = true;
	}

	return true;
}

void EffectAnimationCreate(CoordStruct coord, HouseClass* pHouse)
{
	AnimTypeClass* pAnimType = RulesClass::Instance->ChronoSparkle1;
	if (AnimClass* const pAnim = GameCreate<AnimClass>(pAnimType, coord))
	{
		pAnim->Owner = pHouse;
	}
}

bool CreateTeamChronoW(TActionClass * pThis, HouseClass * pHouse, CellStruct selectedWP)
{
	TaskForceEntryStruct* tStruct = pThis->TeamType->TaskForce->Entries;
	HouseClass* house = pThis->TeamType->Owner;
	int level = pThis->TeamType->VeteranLevel;
	bool full = pThis->TeamType->Full;
	std::vector<TechnoTypeClass*> transports;
	std::vector<FootClass*> foots;
	std::vector<FootClass*> foots2;
	int a[1000][2];
	int maxsize = 0;
	int x = 0;
	int y = 0;
	int xy = 0;
	TeamClass* pTeam = GameCreate<TeamClass>(pThis->TeamType, house, 0);

	if (full)
	{
		// Collect available transports

		for (int i = 0; i < 6; ++i)
		{
			TaskForceEntryStruct currentEntry = tStruct[i];
			FootClass* pFoot;
			if (currentEntry.Amount > 0)
			{
				auto pType = currentEntry.Type;
				if (pType->Passengers > 0)
				{
					transports.emplace_back(pType);
					if (pType->Passengers > maxsize) maxsize = pType->Passengers;
				}
			}
		}
	}
	if (transports.size() > 0)
	{
		for (int i = 0; i < 6; ++i)
		{
			TaskForceEntryStruct currentEntry = tStruct[i];
			FootClass* pFoot;
			if (currentEntry.Amount > 0)
			{
				auto pType = currentEntry.Type;
				if (pType->WhatAmI() == AbstractType::BuildingType) return true;
				if (containsPointer(transports, pType) && pType->Passengers == maxsize)
				{
					for (int j = 0; j < currentEntry.Amount; j++)
					{
						pFoot = static_cast<FootClass*>(pType->CreateObject(house));
						if (level == 2)
						{
							pFoot->Veterancy.SetVeteran();
						}
						else if (level >= 3)
						{
							pFoot->Veterancy.SetElite();
						}
						foots.emplace_back(pFoot);
					}
				}
			}
		}

		for (int i = 0; i < 6; ++i)
		{
			TaskForceEntryStruct currentEntry = tStruct[i];
			FootClass* pFoot;
			if (currentEntry.Amount > 0)
			{
				auto pType = currentEntry.Type;
				if (pType->WhatAmI() == AbstractType::BuildingType) return true;
				if (pType->Passengers < maxsize)
				{
					for (int j = 0; j < currentEntry.Amount; j++)
					{
						pFoot = static_cast<FootClass*>(pType->CreateObject(house));
						if (level == 2)
						{
							pFoot->Veterancy.SetVeteran();
						}
						else if (level >= 3)
						{
							pFoot->Veterancy.SetElite();
						}
						foots2.emplace_back(pFoot);
					}
				}
			}
		}
		std::vector<FootClass*>toDelete;
		auto pCell = MapClass::Instance->TryGetCellAt(selectedWP);
		CoordStruct locat = pCell->GetCoords();
		for (auto pFoot : foots)
		{
			auto techtype = pFoot->GetTechnoType();
			int size = techtype->Passengers;
			int limit = techtype->SizeLimit;
			int sizeleft = size;

			FootClass* pGunner = nullptr;
			bool topped = pFoot->GetTechnoType()->OpenTopped;
			for (auto pUnit : foots2)
			{
				auto utype = pUnit->GetTechnoType();
				if (sizeleft > 0 && utype->Size <= limit && utype->Size <= sizeleft && !containsPointer(toDelete, pUnit))
				{
					pGunner = pUnit;
					if (topped)
					{
						pFoot->EnteredOpenTopped(pUnit);
					}
					pUnit->Transporter = pFoot;
					pFoot->AddPassenger(pUnit);
					pTeam->AddMember(pUnit, true);
					toDelete.push_back(pUnit);
					sizeleft -= utype->Size;
				}
			}
			if (pFoot->GetTechnoType()->Gunner && pGunner)
				pFoot->ReceiveGunner(pGunner);
			bool success;
			auto unit = pFoot->GetTechnoType();
			bool allowBridges = GroundType::Array[static_cast<int>(LandType::Clear)].Cost[static_cast<int>(unit->SpeedType)] > 0.0;
			bool isBridge = allowBridges && pCell->ContainsBridge();
			auto nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(locat),
				unit->SpeedType, -1, unit->MovementZone, isBridge, 1, 1, true,
				false, false, isBridge, CellStruct::Empty, false, false);

			pCell = MapClass::Instance->TryGetCellAt(nCell);
			CoordStruct loc = pCell->GetCoords();

			if (!pCell->GetBuilding())
			{
				++Unsorted::IKnowWhatImDoing;
				success = pFoot->Unlimbo(loc, DirType::NorthEast);
				--Unsorted::IKnowWhatImDoing;
				pTeam->AddMember(pFoot, true);
			}
			else
			{
				success = pFoot->Unlimbo(loc, DirType::NorthEast);
				pTeam->AddMember(pFoot, true);
			}
			EffectAnimationCreate(loc, house);
			AnimTypeClass* pAnimType = RulesClass::Instance->ChronoBlastDest;
			if (AnimClass* const pAnim = GameCreate<AnimClass>(pAnimType, locat))
			{
				pAnim->Owner = pHouse;
			}
		}
		bool suc;
		for (auto pUnit : foots2)
		{
			auto unit = pUnit->GetTechnoType();
			bool allowBridges = GroundType::Array[static_cast<int>(LandType::Clear)].Cost[static_cast<int>(unit->SpeedType)] > 0.0;
			bool isBridge = allowBridges && pCell->ContainsBridge();
			auto nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(locat),
				unit->SpeedType, -1, unit->MovementZone, isBridge, 1, 1, true,
				false, false, isBridge, CellStruct::Empty, false, false);

			pCell = MapClass::Instance->TryGetCellAt(nCell);
			CoordStruct loc = pCell->GetCoords();
			if (!containsPointer(toDelete, pUnit))
			{
				++Unsorted::IKnowWhatImDoing;
				suc = pUnit->Unlimbo(loc, DirType::NorthEast);
				pTeam->AddMember(pUnit, true);
				--Unsorted::IKnowWhatImDoing;
			}
		}
		pTeam->IsForcedActive = true;
		pTeam->CurrentScript = GameCreate<ScriptClass>(pThis->TeamType->ScriptType);
		pTeam->StepCompleted = true;

	}
	//	TeamClass* pTeam = pThis->TeamType->CreateTeam(house);
		// Check if is a valid Waypoint
	else
	{
		for (int i = 0; i < 6; ++i)
		{
			TaskForceEntryStruct currentEntry = tStruct[i];
			if (currentEntry.Amount > 0)
			{
				auto pTechno = currentEntry.Type;
				FootClass* pFoot;
				if (pTechno->WhatAmI() != AbstractType::BuildingType)
				{
					for (int j = 0; j < currentEntry.Amount; j++)
					{
						pFoot = static_cast<FootClass*>(pTechno->CreateObject(house));
						if (level == 2)
						{
							pFoot->Veterancy.SetVeteran();
						}
						else if (level >= 3)
						{
							pFoot->Veterancy.SetElite();
						}
						bool success;
						auto pCell = MapClass::Instance->TryGetCellAt(selectedWP);
						const CoordStruct locat = pCell->GetCoords();


						auto unit = pFoot->GetTechnoType();
						bool allowBridges = GroundType::Array[static_cast<int>(LandType::Clear)].Cost[static_cast<int>(unit->SpeedType)] > 0.0;
						bool isBridge = allowBridges && pCell->ContainsBridge();
						auto nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(locat),
							unit->SpeedType, -1, unit->MovementZone, isBridge, 1, 1, true,
							false, false, isBridge, CellStruct::Empty, false, false);

						pCell = MapClass::Instance->TryGetCellAt(nCell);
						CoordStruct loc = pCell->GetCoords();
						if (!pCell->GetBuilding())
						{
							++Unsorted::IKnowWhatImDoing;
							success = pFoot->Unlimbo(loc, DirType::NorthEast);
							--Unsorted::IKnowWhatImDoing;
							pTeam->AddMember(pFoot, true);

						}
						else
						{
							success = pFoot->Unlimbo(loc, DirType::NorthEast);
							pTeam->AddMember(pFoot, true);
						}
						EffectAnimationCreate(loc, house);
						AnimTypeClass* pAnimType = RulesClass::Instance->ChronoBlastDest;
						if (AnimClass* const pAnim = GameCreate<AnimClass>(pAnimType, locat))
						{
							pAnim->Owner = pHouse;
						}
					}
				}
				else
				{
					return true;
				}
			}
		}
		pTeam->CurrentScript = GameCreate<ScriptClass>(pThis->TeamType->ScriptType);
		pTeam->StepCompleted = true;
	}
	return true;
}

bool TActionExt::CreateTeamChronoToRandomUnit(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pThis)
		return true;
	std::vector<CellStruct> c;
	int j = 0;

	if (RulesExt::Global()->AITargetTypesLists.size() > 0
		&& RulesExt::Global()->AITargetTypesLists[pThis->Waypoint].size() > 0)
	{
		HouseClass* targetHouse = HouseClass::Array->GetItem(pThis->Param3);
		for (auto const pTechno : *TechnoClass::Array())
		{
			if (pTechno->Owner == targetHouse)
			{
				for (auto item : RulesExt::Global()->AITargetTypesLists[pThis->Waypoint])
				{
					if (pTechno->GetTechnoType() == item && !pTechno->InLimbo && pTechno->IsOnMap && pTechno->IsAlive)
					{
						CellStruct cell = { pTechno->GetMapCoords().X ,pTechno->GetMapCoords().Y };
						c.push_back(cell);
						j++;

					}
				}
			}
		}
	}
	if (j == 0) return true;
	int i = ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(c.size())-1);
	CreateTeamChronoW(pThis, pHouse, c[i]);
	return true;
}
bool TActionExt::CreateTeamChrono(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto& waypoints = ScenarioExt::Global()->Waypoints;
	int nWaypoint = pThis->Waypoint;
	CellStruct selectedWP;
	if (nWaypoint >= 0 && waypoints.find(nWaypoint) != waypoints.end() && waypoints[nWaypoint].X && waypoints[nWaypoint].Y)
	{
		selectedWP = waypoints[nWaypoint];
		CreateTeamChronoW(pThis, pHouse, selectedWP);
	}
	else
	{
		return true;
	};
}
bool TActionExt::CreateTeamChronoRandom(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	std::vector<TeamTypeClass*> teamtypes;
	auto& waypoints = ScenarioExt::Global()->Waypoints;
	for (auto pTeamType : *TeamTypeClass::Array())
	{
		if (pThis->Value == pTeamType->Group)
		{
			teamtypes.push_back(pTeamType);
		}
	}
	CellStruct selectedWP;
	int i = ScenarioClass::Instance->Random.RandomRanged(0, teamtypes.size() - 1);
	pThis->TeamType = teamtypes[i];
	selectedWP = waypoints[teamtypes[i]->Waypoint];
	CreateTeamChronoW(pThis, pHouse, selectedWP);
	return true;
}
bool TActionExt::SetTechlevel(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	TechnoTypeClass* techtype= TechnoTypeClass::Find(pThis->Text);
	techtype->TechLevel = pThis->Param3;
	auto aHouse= HouseClass::Array->GetItem(pThis->Param4);
		aHouse->RecheckTechTree = true;
	return true;
}
bool TActionExt::SetTechSpeed(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	TechnoTypeClass* techtype = TechnoTypeClass::Find(pThis->Text);
	techtype->Speed = pThis->Param3;
	return true;
}
bool TActionExt::SetTechWeapon(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto spcialText = pThis->Text;
	const int maxTokens = 2;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbol(spcialText, delimiter, result, maxTokens);
	const auto TechName = result[0];
	const auto WeaponName = result[1];
	TechnoTypeClass* techtype = TechnoTypeClass::Find(TechName);
	WeaponTypeClass* wtype = WeaponTypeClass::Find(WeaponName);
	techtype->Weapon[0].WeaponType = wtype;
	return true;
}
bool TActionExt::SetWeaponRange(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	WeaponTypeClass* wtype = WeaponTypeClass::Find(pThis->Text);
	wtype->Range = pThis->Param3;
	return true;
}
bool TActionExt::SetStrength(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	TechnoTypeClass* techtype = TechnoTypeClass::Find(pThis->Text);

	int y = techtype->Strength;
	techtype->Strength = pThis->Param3;

	for (auto const pTechno : *TechnoClass::Array())
	{
		if (pTechno->GetTechnoType() == techtype)
		{
			if (pThis->Param4 == 1)
			{
				pTechno->Health = pThis->Param3 * (pTechno->Health / y);
				pTechno->EstimatedHealth = pTechno->Health;
			}
			else
			{
				if (pTechno->Health > pThis->Param3)
				{
					pTechno->Health = pThis->Param3;
					pTechno->EstimatedHealth = pTechno->Health;
				}
			}
		}
	}
	return true;
}
void clearFileContent(const std::string& filePath)
{
	std::ofstream ofs(filePath, std::ofstream::out | std::ofstream::trunc);
	if (!ofs)
	{
		// 处理打开文件失败的情况  
		throw std::runtime_error("无法打开文件: " + filePath);
	}

	// 不需要写入任何内容，因为已经使用了trunc模式  
	// 该模式会清除文件的所有内容  
	ofs.close();
}
bool TActionExt::ClearFile(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	try
	{
		clearFileContent(pThis->Text);
	}
	catch (const std::exception& e)
	{
		return false;
	}
	return true;
}
bool TActionExt::BanSaving(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	ScenarioExt::Global()->CanSaveOrLoad = false;
	return true;
}
bool TActionExt::CanSaving(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	ScenarioExt::Global()->CanSaveOrLoad = true;
	return true;
}
bool TActionExt::SetTechLevelByString(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	for (auto b : *TechnoTypeClass::Array)
	{
		std::string strA(b->ID);
		std::string strB(pThis->Text);
		size_t pos = strA.find(strB);
		if (pos != std::string::npos)
		{
			b->TechLevel = pThis->Param3;
		}
	}
	for (auto aHouse : *HouseClass::Array)
	{
		aHouse->RecheckTechTree = true;
	}
	return true;
}
bool TActionExt::NodesInitialize(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	HouseClass* house = HouseClass::Array->GetItem(pThis->Value);
	int i = 0;
	for (auto node : house->Base.BaseNodes)
	{
		CellStruct cell = node.MapCoords;
		bool findit = false;
		for (auto building : *BuildingClass::Array)
		{
			if (building->Owner == house && building->Type->ArrayIndex == node.BuildingTypeIndex)
			{
				auto coord = building->GetMapCoords();
				if (coord.X == node.MapCoords.X && coord.Y == node.MapCoords.Y)
				{
					house->Base.BaseNodes[i].Placed = true;
					house->Base.BaseNodes[i].Attempts = 0;
					findit = true;
					break;

				}
			}
			
		}
		if(!findit)
		{
			house->Base.BaseNodes[i].Placed = false;
			house->Base.BaseNodes[i].Attempts = 0;
		}
		i++;
		findit = true;
	}
	return true;
}
bool TActionExt::CreateAreaClass(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	std::vector<CellStruct> cells;
	int waypoint1=pThis->Param3, waypoint2=pThis->Param4;
	for (int i = waypoint1; i <= waypoint2; i++)
	{
		CellStruct coord = ScenarioClass::Instance()->GetWaypointCoords(i);
		cells.push_back(coord);
	}
	auto newarea=new AreaClass(pThis->Value, cells);
	AreaClass::Array.emplace_back(newarea);
	auto areas= AreaClass::Array;
	for (int i=0;i<areas.size();i++)
	{
		if (areas[i]->ID == pThis->Value)
		{
			break;
		}
	}
	return true;
}
bool TActionExt::OutputAreaClass(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto areas = AreaClass::Array;
	for (int i = 0; i < areas.size(); i++)
	{
		if (areas[i]->ID == 0)
		{
			CRT::swprintf(Phobos::wideBuffer, L"%d", areas[i]->Cells.size());
			MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
			break;
		}
	}
	return true;
}
bool TActionExt::CreateAreaClassByquadrilateral(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	int way1 = pThis->Param3, way2 = pThis->Param4, way3 = pThis->Param5, way4 = pThis->Param6;
	AreaClass* newarea = new AreaClass();
	newarea->ID = pThis->Value;

	newarea->FillQuadrilateral(way1,way2,way3,way4);
	AreaClass::Array.emplace_back(newarea);
	auto areas = AreaClass::Array;
	for (int i = 0; i < areas.size(); i++)
	{
		if (areas[i]->ID == pThis->Value)
		{
			break;
		}
	}
	return true;
}
bool TActionExt::RemoveCellFromAreaClassByWaypoint(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	int way1 = pThis->Param3;
	CellStruct coord = ScenarioClass::Instance()->GetWaypointCoords(way1);
	auto areas = AreaClass::Array;
	for (int i = 0; i < areas.size(); i++)
	{
		if (areas[i]->ID == pThis->Value)
		{
			auto& cells = areas[i]->Cells; 
			cells.erase(
				std::remove_if(
					cells.begin(),
					cells.end(),
					[&coord](const CellStruct& cell)
					{
						return coord.X == cell.X && coord.Y == cell.Y;
					}
				),
				cells.end()
			);
			break;
		}
	}
	return true;
}
bool TActionExt::RemoveCellsFromAreaUnpassable(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto areas = AreaClass::Array;
	for (int i = 0; i < areas.size(); i++)
	{
		if (areas[i]->ID == pThis->Value)
		{
			
			auto& cells = areas[i]->Cells;
			cells.erase(
				std::remove_if(
					cells.begin(),
					cells.end(),
					[](const CellStruct& cell)
					{
						CellClass* pCell = MapClass::Instance->TryGetCellAt(cell);
						return pCell && !pCell->IsClearToMove( // 检查是否不可通行
							SpeedType::Foot,
							false,    // 是否忽略步兵
							false,    // 是否忽略载具
							-1,       // 所属国家（-1 表示任意）
							MovementZone::Infantry, // 移动区域类型
							-1,       
							false     
						);
					}
				),
				cells.end()
			);
			break;
		}
	}
	return true;
}
bool TActionExt::RemoveCellsFromAreaCannotBuild(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto areas = AreaClass::Array;
	HouseClass* house= HouseClass::Array->GetItem(pThis->Param4);
	BuildingTypeClass* pType = BuildingTypeClass::Find(pThis->Text);
	for (int i = 0; i < areas.size(); i++)
	{
		if (areas[i]->ID == pThis->Param3)
		{
			auto& cells = areas[i]->Cells;

			// 使用 erase-remove 惯用法删除无法建造的格子
			cells.erase(
				std::remove_if(
					cells.begin(),
					cells.end(),
					[pType, house](CellStruct& cell)
					{ // 参数改为引用
						return !pType->CanPlaceHere(&cell, house); // 直接取引用地址
					}
				),
				cells.end()
			);
			break;
		}
	}
	return true;

}
bool TActionExt::AddCellToAreaByWaypoint(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto areas = AreaClass::Array;
	CellStruct coord = ScenarioClass::Instance()->GetWaypointCoords(pThis->Param3);
	bool repeat = false;
	for (int i = 0; i < areas.size(); i++)
	{
		if (areas[i]->ID == pThis->Value)
		{
			for (auto cell : areas[i]->Cells)
			{
				if (cell.X == coord.X && cell.Y == coord.Y)
				{
					repeat = true;
					break;
				}
			}
			if (!repeat)
			{
				areas[i]->Cells.push_back(coord);
			}
		}
	}

}
// =============================
// container

TActionExt::ExtContainer::ExtContainer() : Container("TActionClass") { }

TActionExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks
// 函数来分割字符串  

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

#include "Body.h"

#include<ScenarioClass.h>
#include <filesystem>
#include <Ext/Scenario/Body.h>
#include <TlHelp32.h>
#include <HouseClass.h>
#include <RadioClass.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/Template.h>
#include <MessageListClass.h>
#include <random>
#include <AbstractClass.h>
#include <WWMessageBox.h>
#include <Ext/Trigger/Body.h>
/*
用于判断某个单位是否能加入某个小队的判断触发
bool sub_4E0080(FootClass* FirstUnit)
{
	return FirstUnit->ShouldEnterAbsorber || FirstUnit->ShouldEnterOccupiable || FirstUnit->ShouldGarrisonStructure;
}
bool sub_5B36E0(int missionControlIndex)
{
	return missionControlIndex == -1 || MissionControlClass::Array[missionControlIndex].Recruitable;
}
bool sub_6EA610(TeamClass* pThis, FootClass* FirstUnit, int* p_FirstUnit, bool a4)
{
	int v5; // edi
	Mission missionControlIndex; // eax
	ScriptClass* CurrentScript; // ecx
	bool v8; // bl
	TeamClass* Team; // eax
	ScriptActionNode buffer_; // [esp+10h] [ebp-8h] BYREF

	if (pThis == FirstUnit->Team
	  || !FirstUnit
	  || !FirstUnit->IsAlive
	  || !FirstUnit->Health
	  || !Unsorted::IKnowWhatImDoing && FirstUnit->InLimbo
	  || FirstUnit->Owner != pThis->Owner
	  || FirstUnit->HasAnyLink()
	 )
	{
		return false;
	}
	*p_FirstUnit = 0;
	if (pThis->Type->TaskForce->CountEntries > 0)
	{
		do
		{
			v5 = *p_FirstUnit;
			if (pThis->Type->TaskForce->Entries[v5].Type == FirstUnit->GetTechnoType())
				break;
			*p_FirstUnit = v5 + 1;
		}
		while (v5 + 1 < pThis->Type->TaskForce->CountEntries);
	}
	if (*p_FirstUnit == pThis->Type->TaskForce->CountEntries && !a4)
		return 0;
	if (FirstUnit->GetCurrentMission() != Mission::None)
	{
		int missionControlIndex = static_cast<int>(FirstUnit->GetCurrentMission());
		if (!sub_5B36E0(missionControlIndex))
			return false;
	}
	if (!FirstUnit->RecruitableA && !pThis->Type->Autocreate)
		return false;
	CurrentScript = pThis->CurrentScript;
	v8 = false;
	if (CurrentScript)
		v8 = CurrentScript->GetCurrentAction(&buffer_)->Action == 8;
	if (FirstUnit->WhatAmI() == AbstractType::Aircraft)
		v8 = true;
	return (FirstUnit->RecruitableB || !pThis->Type->Autocreate || v8)
		&& !sub_4E0080(FirstUnit)
		&& !FirstUnit->BunkerLinkedItem
		&& ((Team = FirstUnit->Team) == 0 || Team->Type->Priority < pThis->Type->Priority)
		&& (FirstUnit->WhatAmI() != AbstractType::Aircraft
		 || !FirstUnit->GetWeapon(0)->WeaponType
		 || FirstUnit->Ammo)
		&& (pThis->CountObjects[*p_FirstUnit] < pThis->Type->TaskForce->Entries[*p_FirstUnit].Amount || a4)
		&& !FirstUnit->BunkerLinkedItem
		&& FirstUnit->LocomotorSource == 0;
}*/
std::wstring replaceSubstringWithIntegerU(const std::wstring& wstr, const std::wstring& wss, int replacementValue)
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
wchar_t* ConvertWcharN(const char* asciiStr)
{
	size_t len = strlen(asciiStr) + 1; // 包括null终止符  
	wchar_t* wideStr = new wchar_t[len];
	for (size_t i = 0; i < len; ++i)
	{
		wideStr[i] = static_cast<wchar_t>(asciiStr[i]);
	}
	return wideStr;
}
wchar_t* csfConvertU(const wchar_t* wstr)
{
	std::wstring inputStr(wstr);
	std::map<int, ExtendedVariable> variables = ScenarioExt::Global()->Variables[0];
	int i = 0;
	for (const auto& variable : variables)
	{
		std::wstring pattern = L"%var" + std::to_wstring(i) + L"%";
		inputStr = replaceSubstringWithIntegerU(inputStr, pattern, variable.second.Value);
		i++;
	}

	// 转换std::wstring为wchar_t*  
	size_t size = inputStr.size() + 1; // +1 for the null-terminator  
	wchar_t* result = new wchar_t[size];
	std::wcscpy(result, inputStr.c_str());

	return result;
}

bool TActionExt::OutputRealTime(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto a = ScenarioClass::Instance()->ElapsedTimer;
	CRT::swprintf(Phobos::wideBuffer, L"%d", a.CurrentTime);
	MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
	return true;
}
void splitByAtSymbolN(const char* str, char delim, char result[][32], int resultSize)
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
bool TActionExt::SetLevelUnlock(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	int i = 1;
	const auto spcialText = pThis->Text;
	const int maxTokens = 3;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbolN(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto SectionName = result[1];
	const auto ValueName = result[2];//s.ini@TechNum@SHK

	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);
	auto fINI = GameCreate<CCINIClass>();
	auto file = GameCreate<CCFileClass>("m.ini");
	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
	{
		return true;
	}
	char* str = new char[200];
	char* techname = new char[200];
	int laststage;
	std::strcpy(str, "");
	for (i = 1; i < pThis->Param3; i++)//此处参数应该填50
	{
		std::string number = std::to_string(i);
		const char* c_str = number.c_str();
		if (pINI->ReadString(SectionName, c_str, "", Phobos::readBuffer, FixedString<0x200>::Size))
		{
			if (!INIClass::IsBlank(Phobos::readBuffer))
			{
				str = Phobos::readBuffer;
			}
			else
			{
				std::strcpy(str, "");
			}
		}
		if (strcmp(str, ValueName) == 0)
		{
			int j ;
			std::string num;
			const char* c_str2;
			for (j = 2; j <= pThis->Param3; j++)
			{
				num = std::to_string(j);
				c_str2 = num.c_str();
				laststage = pINI->ReadInteger("UnlockTree", c_str2, 0);
				if (laststage == i)
				{
					if (pINI->ReadString(SectionName, c_str2, "", Phobos::readBuffer, FixedString<0x200>::Size))
					{
						if (!INIClass::IsBlank(Phobos::readBuffer))
						{
							techname = Phobos::readBuffer;
							//CRT::swprintf(Phobos::wideBuffer, L"%s", ConvertWcharN(techname));
							//MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
						}
						else
						{
							std::strcpy(techname, "");
						}
					}
					if (file->Exists())
						fINI->ReadCCFile(file);
					else
					{
						return true;
					}
					fINI->WriteInteger("Stage", techname, 1, false);
					fINI->WriteCCFile(file);
				}

			}
		}
	}
	file->Close();
	pFile->Close();
	return true;
}
bool TActionExt::RandomTechNum(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto spcialText = pThis->Text;
	const int maxTokens = 3;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbolN(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto SectionName = result[1];
	const auto TechListSection = result[2];//s.ini@TechNum2@TechNum
	int i = 1;

	std::srand(static_cast<unsigned int>(std::time(0)));

	std::vector<std::string> lines;
	std::string line;

	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);
	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
	{
		pFile->CreateFileA();
	}
	for (i = 1; i <= pThis->Param3; i++)//此处参数应该填50
	{
		std::string number = std::to_string(i);
		const char* c_str = number.c_str();
		std::string sr;
		if (pINI->ReadString(TechListSection, c_str, "", Phobos::readBuffer, FixedString<0x200>::Size))
		{
			if (!INIClass::IsBlank(Phobos::readBuffer))
			{
				sr = Phobos::readBuffer;
				lines.push_back(sr);
			}
			else
			{
				sr = "";
			}
		}
	}
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(lines.begin(), lines.end(), g);
	i = 1;
	for (auto s : lines)
	{
		std::string number = std::to_string(i);
		const char* c_str = number.c_str();
		pINI->WriteString(SectionName, c_str, s.c_str());
		i++;
	}
	pINI->WriteCCFile(pFile);
	pFile->Close();
	return true;
}
bool TActionExt::SetFirstTechUnlock(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto spcialText = pThis->Text;
	const int maxTokens = 3;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbolN(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto SectionName = result[1];
	const auto fileName2 = result[2];//s.ini@TechNum@m.ini
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);
	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
	{
		pFile->CreateFileA();
	}
	auto pINI2 = GameCreate<CCINIClass>();
	auto pFile2 = GameCreate<CCFileClass>(fileName2);
	if (pFile2->Exists())
		pINI2->ReadCCFile(pFile2);
	else
	{
		pFile2->CreateFileA();
	}
	std::string number = std::to_string(pThis->Param3);
	const char* c_str = number.c_str();
	char* tech = new char[200];
	if (pINI->ReadString(SectionName, c_str, "", Phobos::readBuffer, FixedString<0x200>::Size))
	{
		if (!INIClass::IsBlank(Phobos::readBuffer))
		{
			tech = Phobos::readBuffer;
		}
		else
		{
			std::strcpy(tech, "");
		}
	}
	pINI2->WriteInteger("Stage", tech, 1, false);
	pINI2->WriteInteger("Unlock", tech, 0, false);
	pINI2->WriteCCFile(pFile2);
	pFile2->Close();
	pFile->Close();
	return true;
}
bool TActionExt::UpdateTeamDelays(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	int hard = pThis->Value;
	int normal = pThis->Param3;
	int easy = pThis->Param4;
	RulesClass::Instance->TeamDelays[0] = hard;
	RulesClass::Instance->TeamDelays[1] = normal;
	RulesClass::Instance->TeamDelays[2] = easy;
	return true;
}

void updateArmor1(HouseClass* pHouse, int multype, double numd)
{
	float num = static_cast<float>(numd);
	if (multype == 0)
	{
		pHouse->Type->ArmorBuildingsMult = num;
	}
	else if (multype == 1)
	{
		pHouse->Type->ArmorUnitsMult = num;
	}
	else if (multype == 2)
	{
		pHouse->Type->ArmorInfantryMult = num;
	}
	else if (multype == 3)
	{
		pHouse->Type->ArmorAircraftMult = num;
	}
	else if (multype == 4)
	{
		pHouse->Type->ArmorDefensesMult = num;
	}
	else
	{
		pHouse->Type->ArmorMult = num;
	}
}
bool TActionExt::UpdateArmor(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	int multype = pThis->Param3;
	double num = std::stod(pThis->Text);
	updateArmor1(pHouse, multype, num);
	return true;
}
bool TActionExt::UpdateArmorCsf(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto spcialText = pThis->Text;
	int multype = pThis->Param3;
	const int maxTokens = 3;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbolN(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto Section = result[1];
	const auto KeyName = result[2];
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);

	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
		pFile->CreateFileA();
	double num= pINI->ReadDouble(Section, KeyName, 0);
	updateArmor1(pHouse, multype, num);
	return true;
}
void UpdateBuildingSpeed1(HouseClass* pHouse, int multype, double num1)
{
	float num = static_cast<float>(num1);
	if (multype == 0)
	{
		pHouse->Type->BuildtimeBuildingsMult = num;
	}
	else if (multype == 1)
	{
		pHouse->Type->BuildtimeUnitsMult = num;
	}
	else if (multype == 2)
	{
		pHouse->Type->BuildtimeInfantryMult = num;
	}
	else if (multype == 3)
	{
		pHouse->Type->BuildtimeAircraftMult = num;
	}
	else if (multype == 4)
	{
		pHouse->Type->BuildtimeDefensesMult = num;
	}
	else
	{
		pHouse->Type->BuildtimeMult = num;
	}
}
bool TActionExt::UpdateBuildingSpeed(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	int multype = pThis->Param3;
	double num = std::stod(pThis->Text);
	return true;
}
bool TActionExt::UpdateBuildingSpeedCsf(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto spcialText = pThis->Text;
	int multype = pThis->Param3;
	const int maxTokens = 3;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbolN(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto Section = result[1];
	const auto KeyName = result[2];
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);

	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
		pFile->CreateFileA();
	double num = pINI->ReadDouble(Section, KeyName, 0);	
	UpdateBuildingSpeed1(pHouse, multype, num);
	return true;
}
void UpdateFirePower1(HouseClass* pHouse, double num1)
{
	pHouse->Type->FirepowerMult = num1;
}
bool TActionExt::UpdateFirePower(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	double num = std::stod(pThis->Text);
	UpdateFirePower1(pHouse, num);
	return true;
}
bool TActionExt::UpdateFirePowerCsf(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto spcialText = pThis->Text;
	const int maxTokens = 3;
	char result[maxTokens][32];
	char delimiter = '@';
	splitByAtSymbolN(spcialText, delimiter, result, maxTokens);
	const auto fileName = result[0];
	const auto Section = result[1];
	const auto KeyName = result[2];
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);

	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
		pFile->CreateFileA();
	double num = pINI->ReadDouble(Section, KeyName, 0);
	UpdateFirePower1(pHouse, num);
	return true;
}
bool TActionExt::RemoveNodeByTechName(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto techtype = pThis->Text;
	auto Nodes = pHouse->Base.BaseNodes;
	std::vector<BaseNodeClass> nodetypes;
	for (auto node : Nodes)
	{
		int index = node.BuildingTypeIndex;
		if (index >= 0)
		{
			auto pType = TechnoTypeClass::GetByTypeAndIndex(AbstractType::BuildingType, index);
			if (strcmp(pType->ID, techtype) == 0)
			{
				nodetypes.push_back(node);
			}
		}
	}
	for (auto node : nodetypes)
	{
	    pHouse->Base.BaseNodes.Remove(node);
	}
	return true;
}
bool TActionExt::MoveNodeByIDRandom(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	int id = pThis->Value;
	HouseClass* house = HouseClass::Array->GetItem(pThis->Param3);
	CellStruct cell = house->Base.BaseNodes[id].MapCoords;
	SpeedType speedtype;
	MovementZone zone;
	auto techtype=TechnoTypeClass::GetByTypeAndIndex(AbstractType::BuildingType, house->Base.BaseNodes[id].BuildingTypeIndex);
	BuildingTypeClass* pType = BuildingTypeClass::Find(techtype->ID);
	if (pType->Naval)
	{

		speedtype = SpeedType::Float;
		zone = MovementZone::Water;
	}
	else
	{
		speedtype = SpeedType::Foot;
		zone = MovementZone::Normal;
	}
	auto cellStruct = MapClass::Instance->NearByLocation(cell, speedtype,-1, zone, false, pType->GetFoundationWidth(), pType->GetFoundationHeight(false), false, false, false, true, CellStruct::Empty, false, true);
	house->Base.BaseNodes[id].MapCoords = cellStruct;
	house->Base.BaseNodes[id].Attempts = 0;
	return true;
}
bool TActionExt::MoveNodeByIDWaypoint(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	int id = pThis->Value;
	int waypoint = pThis->Param3;
	HouseClass* house = HouseClass::Array->GetItem(pThis->Param4);
	CellStruct coord = ScenarioClass::Instance()->GetWaypointCoords(waypoint);
	house->Base.BaseNodes[id].MapCoords = coord;
	house->Base.BaseNodes[id].Attempts = 0;
	return true;
}
bool TActionExt::AddNodeOnWaypoint(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const char* techid = pThis->Text;
	auto pType = BuildingTypeClass::Find(techid);
	int waypoint = pThis->Param3;
	HouseClass* house = HouseClass::Array->GetItem(pThis->Param4);
	CellStruct coord = ScenarioClass::Instance()->GetWaypointCoords(waypoint);
	BaseNodeClass* node = new BaseNodeClass();
	node->Attempts = 0;
	node->MapCoords = coord;
	node->Placed = 0;
	node->BuildingTypeIndex = pType->ArrayIndex;
	house->Base.BaseNodes.AddItem(*node);
	return true;
}
bool TActionExt::AddNodeOnWaypointWithIndex(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const char* techid = pThis->Text;
	auto pType = BuildingTypeClass::Find(techid);
	int waypoint = pThis->Param3;
	HouseClass* house = HouseClass::Array->GetItem(pThis->Param4);
	int index = pThis->Param5;
	CellStruct coord = ScenarioClass::Instance()->GetWaypointCoords(waypoint);
	BaseNodeClass* node = new BaseNodeClass();
	node->Attempts = 0;
	node->MapCoords = coord;
	node->Placed = 0;
	node->BuildingTypeIndex = pType->ArrayIndex;
	house->Base.BaseNodes.AddItem(*node);
	for (int i = house->Base.BaseNodes.Count; i > index; --i)
	{
		house->Base.BaseNodes[i] = house->Base.BaseNodes[i - 1]; // 将元素后移一位
	}
	house->Base.BaseNodes[index] = *node;
	return true;
}
bool TActionExt::MoveNodeByTech(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const char* techid = pThis->Text;
	auto pType = BuildingTypeClass::Find(techid);
	HouseClass* house = HouseClass::Array->GetItem(pThis->Param3);
	SpeedType speedtype;
	MovementZone zone;
	for (int i = 0; i < house->Base.BaseNodes.Count; i++)
	{
		CellStruct cell = house->Base.BaseNodes[i].MapCoords;
		CellClass* pCell= MapClass::Instance->TryGetCellAt(cell);
		if (!pCell)
			continue;
		/*if (i >= 108 && pThis->Param3 == 2)
		{
			if (house->Base.BaseNodes[i].Placed)
			{
				CRT::swprintf(Phobos::wideBuffer, L"Success1=%d", house->Base.BaseNodes[i].BuildingTypeIndex);
				MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
			}
			if (!house->Base.BaseNodes[i].Placed)
			{
				CRT::swprintf(Phobos::wideBuffer, L"failure1=%d", house->Base.BaseNodes[i].BuildingTypeIndex);
				MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
			}
			if (pType->CanPlaceHere(&cell, house))
			{
				CRT::swprintf(Phobos::wideBuffer, L"Success2=%d", house->Base.BaseNodes[i].BuildingTypeIndex);
				MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
			}
			if (!pType->CanPlaceHere(&cell, house))
			{
				CRT::swprintf(Phobos::wideBuffer, L"failure2=%d", house->Base.BaseNodes[i].BuildingTypeIndex);
				MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
			}
		}*/
		if (pCell->GetBuilding()!=nullptr)
		{
			if (pCell->GetBuilding()->GetTechnoType()->GetArrayIndex() == house->Base.BaseNodes[i].BuildingTypeIndex) continue;
		}
		/*if (pThis->Param3 == 2)
		{
			Debug::Log("A:NodeID=%d,coords=(%d,%d),BuildingTypeIndex=%d,GetArrayIndex=%d,Attempts=%d\n", i, house->Base.BaseNodes[i].MapCoords.X, house->Base.BaseNodes[i].MapCoords.Y, house->Base.BaseNodes[i].BuildingTypeIndex, pType->GetArrayIndex(), house->Base.BaseNodes[i].Attempts);
		}*/
		if ((pType->GetArrayIndex() == house->Base.BaseNodes[i].BuildingTypeIndex)&&(!pType->CanPlaceHere(&cell,house)))
		{
			bool buildable;
			if (pType->Naval)
			{
				speedtype = SpeedType::Float;
				zone = MovementZone::Water;
				buildable = false;
				house->NodeLogic_1F0 = true;
			}
			else
			{
				speedtype = SpeedType::Foot;
				zone = MovementZone::Normal;
				buildable = true;
			}

			auto cellStruct = MapClass::Instance->NearByLocation(cell, speedtype, -1, zone, false, pType->GetFoundationWidth(), pType->GetFoundationHeight(false), false, false, false, true, CellStruct::Empty, false, buildable);
			// 在 NearByLocation 返回后,CanPlaceHere 之外加一层检查
			/*auto checkBlocking = reinterpret_cast<int(__thiscall*)(BuildingTypeClass*, CellStruct*, HouseClass*)>(0x45EE70);
			int blockResult = checkBlocking(pType, &cellStruct, house);
			if (pThis->Param3 == 2)
			{
				if (pType->CanPlaceHere(&cellStruct, house))
				{
					Debug::Log("coords=(%d,%d) Canplace,blockResult=%d\n", cellStruct.X, cellStruct.Y, blockResult);
				}
				Debug::Log("B:NodeID=%d,coords=(%d,%d),BuildingTypeIndex=%d,GetArrayIndex=%d,Attempts=%d\n", i, cellStruct.X, cellStruct.Y, house->Base.BaseNodes[i].BuildingTypeIndex, pType->GetArrayIndex(), house->Base.BaseNodes[i].Attempts);
			}*/
			house->Base.BaseNodes[i].MapCoords = cellStruct;
			house->Base.BaseNodes[i].Attempts = 0;
		}
	}
	return true;
}
bool TActionExt::CreateTeamWithBindingUnits(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	TeamClass* pTeam = GameCreate<TeamClass>(pThis->TeamType, pThis->TeamType->Owner, 0);
	for (auto const pTechno : *TechnoClass::Array())
	{

		if (!pTechno) continue;
		if (pTechno->AttachedTag&& pTechno->WhatAmI() != AbstractType::BuildingType && !pTechno->Owner->IsNeutral())
		{
			TriggerClass* trigger = pTechno->AttachedTag->FirstTrigger;
			while (trigger->Type == pTrigger->Type && trigger->NextTrigger)
			{
				trigger = trigger->NextTrigger;
			}
			if (trigger->Type == pTrigger->Type)
			{
				auto pFoot = static_cast<FootClass*>(pTechno);
				if (pFoot->IsAlive && !pFoot->InLimbo && !pTechno->Owner->IsHumanPlayer)
				{
					pFoot->QueueMission(Mission::Guard, true);
					pTeam->Owner = pTechno->Owner;
					++Unsorted::IKnowWhatImDoing;
					pTeam->AddMember(pFoot, true);
					--Unsorted::IKnowWhatImDoing;
				}
			}
		}
	}
	pTeam->IsForcedActive = true;//某些转变所属方的情况下需要使用此语句
	pTeam->CurrentScript = GameCreate<ScriptClass>(pThis->TeamType->ScriptType);
	pTeam->StepCompleted = true;
	return true;
}
bool TActionExt::CreateSingleTeamWithBindingUnits(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	for (auto const pTechno : *TechnoClass::Array())
	{

		if (!pTechno) continue;
		if (pTechno->AttachedTag && pTechno->WhatAmI() != AbstractType::BuildingType && !pTechno->Owner->IsNeutral())
		{
			TriggerClass* trigger = pTechno->AttachedTag->FirstTrigger;
			while (trigger->Type == pTrigger->Type && trigger->NextTrigger)
			{
				trigger = trigger->NextTrigger;
			}
			if (trigger->Type == pTrigger->Type)
			{
				auto pFoot = static_cast<FootClass*>(pTechno);
				if (pFoot->IsAlive && !pFoot->InLimbo&&!pFoot->BelongsToATeam()&& !pTechno->Owner->IsHumanPlayer)
				{
					TeamClass* pTeam = GameCreate<TeamClass>(pThis->TeamType, pTechno->Owner, 0);
					pFoot->QueueMission(Mission::Guard, true);
					++Unsorted::IKnowWhatImDoing;
					pTeam->AddMember(pFoot, true);
					--Unsorted::IKnowWhatImDoing;
					pTeam->IsForcedActive = true;//某些转变所属方的情况下需要使用此语句
					pTeam->CurrentScript = GameCreate<ScriptClass>(pThis->TeamType->ScriptType);
					pTeam->StepCompleted = true;
				}
			}
		}
	}
	return true;

}
bool TActionExt::GetOutWWMessageBox(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	
	while (true)
	{
		// WWMessageBox buttons look like below:
		// Button1
		// Button3
		// Button2
		// I prefer to put the loadgame to the center of them - secsome
		// Did you??? NO, YOU DIDN'T. Bruhhhh
		switch (WWMessageBox::Instance->Process(
			GeneralUtils::LoadStringUnlessMissing("TXT_HARDCORE_NOSAVE", L"Testing"),
			StringTable::LoadString(GameStrings::TXT_OK),
			StringTable::LoadString(GameStrings::TXT_CANCEL),
			StringTable::LoadString(GameStrings::TXT_CONTROL)))
		{
		case WWMessageBox::Result::Button3:
			CRT::swprintf(Phobos::wideBuffer, L"%d", 3);
			MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
			break;
		case WWMessageBox::Result::Button2:
			CRT::swprintf(Phobos::wideBuffer, L"%d", 2);
			MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
			break;

		case WWMessageBox::Result::Button1:
			CRT::swprintf(Phobos::wideBuffer, L"%d", 1);
			MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
			break;
		default:
			break;
		}

		break;
	}
	return true;
}
bool TActionExt::SetTriggerExtCell(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	for (auto const trigger : *TriggerClass::Array())
	{
		if (trigger->Type == pThis->TriggerType)
		{
			const auto pExt = TriggerExt::ExtMap.Find(trigger);
			pExt->Cell.GetEx()->X = pThis->Param3;
			pExt->Cell.GetEx()->Y = pThis->Param4;
			break;
		}
	}
	
	return true;
}
bool TActionExt::PrintTriggerExtCell(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto pExt = TriggerExt::ExtMap.Find(pTrigger);
	CellStruct cell = pExt->Cell;
	CRT::swprintf(Phobos::wideBuffer, L"X=%d,Y=%d", cell.X, cell.Y);
	MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);

	return true;
}


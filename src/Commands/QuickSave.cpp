#include "QuickSave.h"

#include <ScenarioClass.h>
#include <HouseClass.h>
#include <SessionClass.h>
#include <Utilities/GeneralUtils.h>
#include <Ext/Scenario/Body.h>

const char* QuickSaveCommandClass::GetName() const
{
	return "Quicksave";
}

const wchar_t* QuickSaveCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_QUICKSAVE", L"Quicksave");
}

const wchar_t* QuickSaveCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* QuickSaveCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_QUICKSAVE_DESC", L"Save the current game (Singleplayer only).");
}

void QuickSaveCommandClass::Execute(WWKey eInput) const
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

	if (Phobos::Config::NoSaveLoad || !ScenarioExt::Global()->CanSaveOrLoad)
	{
		PrintMessage(StringTable::LoadString(GameStrings::TXT_ERROR_SAVING_GAME));
		return;
	}
	if (SessionClass::IsSingleplayer())
	{
		char fName[0x80];

		SYSTEMTIME time;
		GetLocalTime(&time);

		_snprintf_s(fName, 0x7F, "Map.%04u%02u%02u-%02u%02u%02u-%05u.sav",
			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

		PrintMessage(StringTable::LoadString(GameStrings::TXT_SAVING_GAME));

		wchar_t fDescription[0x80] = { 0 };
		if (SessionClass::IsCampaign())
			wcscpy_s(fDescription, ScenarioClass::Instance->UINameLoaded);
		else
			wcscpy_s(fDescription, ScenarioClass::Instance->Name);
		wcscat_s(fDescription, L" - ");
		wcscat_s(fDescription, GeneralUtils::LoadStringUnlessMissing("TXT_QUICKSAVE_SUFFIX", L"Quicksaved"));

		if (ScenarioClass::SaveGame(fName, fDescription))
			PrintMessage(StringTable::LoadString(GameStrings::TXT_GAME_WAS_SAVED));
		else
			PrintMessage(StringTable::LoadString(GameStrings::TXT_ERROR_SAVING_GAME));
	}
	else
	{
		PrintMessage(StringTable::LoadString("MSG:NotAvailableInMultiplayer"));
	}
}

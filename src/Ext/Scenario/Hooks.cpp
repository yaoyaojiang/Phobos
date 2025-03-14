#include <Ext/Scenario/Body.h>
#include <Helpers/Macro.h>
#include <Utilities/Debug.h>

DEFINE_HOOK(0x6870D7, ReadScenario_LoadingScreens, 0x5)
{
	enum { SkipGameCode = 0x6873AB };

	LEA_STACK(CCINIClass*, pINI, STACK_OFFSET(0x174, -0x158));

	auto const pScenario = ScenarioClass::Instance.get();
	auto const scenarioName = pScenario->FileName;
	auto const defaultsSection = "Defaults";

	pScenario->LS640BriefLocX = pINI->ReadInteger(scenarioName, "LS640BriefLocX", pINI->ReadInteger(defaultsSection, "DefaultLS640BriefLocX", 0));
	pScenario->LS640BriefLocY = pINI->ReadInteger(scenarioName, "LS640BriefLocY", pINI->ReadInteger(defaultsSection, "DefaultLS640BriefLocY", 0));
	pScenario->LS800BriefLocX = pINI->ReadInteger(scenarioName, "LS800BriefLocX", pINI->ReadInteger(defaultsSection, "DefaultLS800BriefLocX", 0));
	pScenario->LS800BriefLocY = pINI->ReadInteger(scenarioName, "LS800BriefLocY", pINI->ReadInteger(defaultsSection, "DefaultLS800BriefLocY", 0));
	pINI->ReadString(defaultsSection, "DefaultLS640BkgdName", pScenario->LS640BkgdName, pScenario->LS640BkgdName, 64);
	pINI->ReadString(scenarioName, "LS640BkgdName", pScenario->LS640BkgdName, pScenario->LS640BkgdName, 64);
	pINI->ReadString(defaultsSection, "DefaultLS800BkgdName", pScenario->LS800BkgdName, pScenario->LS800BkgdName, 64);
	pINI->ReadString(scenarioName, "LS800BkgdName", pScenario->LS800BkgdName, pScenario->LS800BkgdName, 64);
	pINI->ReadString(defaultsSection, "DefaultLS800BkgdPal", pScenario->LS800BkgdPal, pScenario->LS800BkgdPal, 64);
	pINI->ReadString(scenarioName, "LS800BkgdPal", pScenario->LS800BkgdPal, pScenario->LS800BkgdPal, 64);

	return SkipGameCode;
}
DEFINE_HOOK(0x5AE192, SelectNextMission, 0x6)
{
	if (ScenarioExt::Global()->NextMission.isset())
		R->EAX(ScenarioExt::Global()->NextMission.Get().data());

	return 0;
}
DEFINE_HOOK(0x5CF04F, CheckMissionCount, 0x5)
{
	GET(int, number, EDI);
	if (number > 10000)
	{
		return 0x5CF071;
	}
	return 0x5CF054;
}

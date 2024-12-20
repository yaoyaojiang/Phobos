#pragma once

#include <Utilities/Container.h>
#include <Utilities/Template.h>
#include <ScriptClass.h>

#include <Helpers/Template.h>

#include <TActionClass.h>

class HouseClass;

enum class PhobosTriggerAction : unsigned int
{
	SaveGame = 500,
	EditVariable = 501,
	GenerateRandomNumber = 502,
	PrintVariableValue = 503,
	BinaryOperation = 504,
	RunSuperWeaponAtLocation = 505,
	RunSuperWeaponAtWaypoint = 506,
	ToggleMCVRedeploy = 510,
	OutputVariablesToFile=700,
	ReadVariablesFromFile=701,
	OutputStringToFile = 702,
	VariableAssignment=703,
	FreeControlledUnit=704,
	WriteMessage = 705,
	UpdateMaxMessageCount = 706,
	UpdateMessageBuffer=707,
	BindTeamMemberToTag = 708,
	BindTechnologyToTag=709,
	UpdateMoney = 710,
	UpdateNextScenario = 711,
	CreateBannerLocal = 712, // any banner w/ global variable
	DeleteBanner = 713, // any banner w/ local variable
	DisableTriggerrWithMark = 714,
	RunSuperWeaponAtRandomUnit = 715,
	EnableTriggerWithMark=716,
	EnableTriggerWithMarkRandomly=717,
	CreateTeam=718,
	CreateTeamWithGroup=719,
	CreateTeamWithGroupRandomly = 720,
	ClearFile=721,
	OutputInteger = 722,
	OutputIntegerRandomly = 723,
	OutputIntegerRandomlyTargets = 724,
	OutputDouble = 725,
	OutputDoubleWithVar = 726,
	EditOuterIntegers=727,
	EditOuterDoubles = 728,
	EditOuterStrings=729,
};

class TActionExt
{
public:
	using base_type = TActionClass;

	static constexpr DWORD Canary = 0x91919191;

	class ExtData final : public Extension<TActionClass>
	{
	public:
		ExtData(TActionClass* const OwnerObject) : Extension<TActionClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static bool Execute(TActionClass* pThis, HouseClass* pHouse,
			ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location, bool& bHandled);

#pragma push_macro("ACTION_FUNC")
#define ACTION_FUNC(name) \
	static bool name(TActionClass* pThis, HouseClass* pHouse, \
		ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)

	ACTION_FUNC(PlayAudioAtRandomWP);
	ACTION_FUNC(SaveGame);
	ACTION_FUNC(EditVariable);
	ACTION_FUNC(GenerateRandomNumber);
	ACTION_FUNC(PrintVariableValue);
	ACTION_FUNC(BinaryOperation);
	ACTION_FUNC(RunSuperWeaponAtLocation);
	ACTION_FUNC(RunSuperWeaponAtWaypoint);
	ACTION_FUNC(ToggleMCVRedeploy);
	ACTION_FUNC(OutputVariablesToFile);
	ACTION_FUNC(ReadVariablesFromFile);
	ACTION_FUNC(OutputStringToFile);
	ACTION_FUNC(VariableAssignment);
	ACTION_FUNC(FreeControlledUnit);
	ACTION_FUNC(WriteMessage);
	ACTION_FUNC(UpdateMaxMessageCount);
	ACTION_FUNC(UpdateMessageBuffer);
	ACTION_FUNC(BindTeamMemberToTag);
	ACTION_FUNC(BindTechnologyToTag);
	ACTION_FUNC(UpdateMoney);
	ACTION_FUNC(UpdateNextScenario);
	ACTION_FUNC(DisableTriggerrWithMark);
	ACTION_FUNC(CreateBannerLocal);
	ACTION_FUNC(DeleteBanner);
	ACTION_FUNC(RunSuperWeaponAtRandomUnit);
	ACTION_FUNC(EnableTriggerWithMark);
	ACTION_FUNC(EnableTriggerWithMarkRandomly);
	ACTION_FUNC(CreateTeam);
	ACTION_FUNC(CreateTeamWithGroup);
	ACTION_FUNC(CreateTeamWithGroupRandomly);
	ACTION_FUNC(ClearFile);
	ACTION_FUNC(OutputInteger);
	ACTION_FUNC(OutputIntegerRandomly);
	ACTION_FUNC(OutputIntegerRandomlyTargets);
	ACTION_FUNC(OutputDouble);
	ACTION_FUNC(OutputDoubleWithVar);
	ACTION_FUNC(EditOuterIntegers);
	ACTION_FUNC(EditOuterDoubles);
	ACTION_FUNC(EditOuterStrings);
	static bool RunSuperWeaponAt(TActionClass* pThis, int X, int Y);

#undef ACTION_FUNC
#pragma pop_macro("ACTION_FUNC")

	class ExtContainer final : public Container<TActionExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};

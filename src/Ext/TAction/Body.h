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
	OutputDoubleWithVarCsf = 730,
	EditOuterInteger = 731,
	SetTechlevel=732,
	SetTechSpeed=733,
	SetWeaponRange = 734,
	CreateTeamChrono = 735,
	CreateTeamChronoToRandomUnit = 736,
	RunSuperWeaponAtRandomUnit2 = 737,
	CreateTeamChronoRandom = 738,
	MissionFail=739,
	BanSaving=740,
	CanSaving=741,
	OutputRealTime=742,
	SetTechLevelByString=743,
	UpdateBuildingSpeed=744,
	SetLevelUnlock=745,
	RandomTechNum=746,
	SetFirstTechUnlock=747,
	UpdateArmor=748,
	UpdateTeamDelays=749,
	UpdateBuildingSpeedCsf=750,
	UpdateArmorCsf = 751,
	UpdateFirePower = 752,
	UpdateFirePowerCsf = 753,
	SetStrength =754,
	SetTechWeapon=755,
	RemoveNodeByTechName =756,
	MoveNodeByIDRandom = 757,
	MoveNodeByIDWaypoint = 758,
	NodesInitialize=759,
	AddNodeOnWaypoint=760,
	AddNodeOnWaypointWithIndex=761,
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
	ACTION_FUNC(OutputIntegerRandomly);
	ACTION_FUNC(OutputIntegerRandomlyTargets);
	ACTION_FUNC(OutputDouble);
	ACTION_FUNC(OutputDoubleWithVar);
	ACTION_FUNC(EditOuterIntegers);
	ACTION_FUNC(EditOuterDoubles);
	ACTION_FUNC(EditOuterStrings);
	ACTION_FUNC(OutputDoubleWithVarCsf);
	ACTION_FUNC(EditOuterInteger);
	ACTION_FUNC(SetTechlevel);
	ACTION_FUNC(SetTechSpeed);
	ACTION_FUNC(SetWeaponRange);
	ACTION_FUNC(OutputInteger);
	ACTION_FUNC(CreateTeamChrono);
	ACTION_FUNC(CreateTeamChronoToRandomUnit);
	ACTION_FUNC(RunSuperWeaponAtRandomUnit2);
	ACTION_FUNC(CreateTeamChronoRandom);
	ACTION_FUNC(MissionFail);
	ACTION_FUNC(BanSaving);
	ACTION_FUNC(CanSaving);
	ACTION_FUNC(OutputRealTime);
	ACTION_FUNC(SetTechLevelByString);
	ACTION_FUNC(UpdateBuildingSpeed);
	ACTION_FUNC(SetLevelUnlock);
	ACTION_FUNC(RandomTechNum);
	ACTION_FUNC(SetFirstTechUnlock);
	ACTION_FUNC(UpdateArmor);
	ACTION_FUNC(UpdateTeamDelays);
	ACTION_FUNC(UpdateBuildingSpeedCsf);
	ACTION_FUNC(UpdateArmorCsf);
	ACTION_FUNC(UpdateFirePower);
	ACTION_FUNC(UpdateFirePowerCsf);
	ACTION_FUNC(SetStrength);
	ACTION_FUNC(SetTechWeapon);
	ACTION_FUNC(RemoveNodeByTechName);
	ACTION_FUNC(MoveNodeByIDRandom);
	ACTION_FUNC(MoveNodeByIDWaypoint);
	ACTION_FUNC(NodesInitialize);
	ACTION_FUNC(AddNodeOnWaypoint);
	ACTION_FUNC(AddNodeOnWaypointWithIndex);
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

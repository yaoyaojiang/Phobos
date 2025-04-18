#pragma once

#include <Utilities/Container.h>
#include <Utilities/Template.h>

#include <Helpers/Template.h>

#include <TEventClass.h>

class HouseClass;

enum PhobosTriggerEvent
{
	LocalVariableGreaterThan = 500,
	LocalVariableLessThan = 501,
	LocalVariableEqualsTo = 502,
	LocalVariableGreaterThanOrEqualsTo = 503,
	LocalVariableLessThanOrEqualsTo = 504,
	LocalVariableAndIsTrue = 505,
	GlobalVariableGreaterThan = 506,
	GlobalVariableLessThan = 507,
	GlobalVariableEqualsTo = 508,
	GlobalVariableGreaterThanOrEqualsTo = 509,
	GlobalVariableLessThanOrEqualsTo = 510,
	GlobalVariableAndIsTrue = 511,
	LocalVariableGreaterThanLocalVariable = 512,
	LocalVariableLessThanLocalVariable = 513,
	LocalVariableEqualsToLocalVariable = 514,
	LocalVariableGreaterThanOrEqualsToLocalVariable = 515,
	LocalVariableLessThanOrEqualsToLocalVariable = 516,
	LocalVariableAndIsTrueLocalVariable = 517,
	GlobalVariableGreaterThanLocalVariable = 518,
	GlobalVariableLessThanLocalVariable = 519,
	GlobalVariableEqualsToLocalVariable = 520,
	GlobalVariableGreaterThanOrEqualsToLocalVariable = 521,
	GlobalVariableLessThanOrEqualsToLocalVariable = 522,
	GlobalVariableAndIsTrueLocalVariable = 523,
	LocalVariableGreaterThanGlobalVariable = 524,
	LocalVariableLessThanGlobalVariable = 525,
	LocalVariableEqualsToGlobalVariable = 526,
	LocalVariableGreaterThanOrEqualsToGlobalVariable = 527,
	LocalVariableLessThanOrEqualsToGlobalVariable = 528,
	LocalVariableAndIsTrueGlobalVariable = 529,
	GlobalVariableGreaterThanGlobalVariable = 530,
	GlobalVariableLessThanGlobalVariable = 531,
	GlobalVariableEqualsToGlobalVariable = 532,
	GlobalVariableGreaterThanOrEqualsToGlobalVariable = 533,
	GlobalVariableLessThanOrEqualsToGlobalVariable = 534,
	GlobalVariableAndIsTrueGlobalVariable = 535,
	LocalVariableNotEqualsTo=536,
	LocalVariableNotEqualsToLocalVariable=537,
	ShieldBroken = 600,
	HouseOwnsTechnoType = 601,
	HouseDoesntOwnTechnoType = 602,
	SuperWeaponTimerUp=603,
	OwnerApproachWaypoint=604,
	ContainsShroud = 605,
	Cheating = 606,
	OuterVariableEqual=607,
	OuterVariableBigger = 608,
	OuterVariableSmaller = 609,
	OuterVariableNotEqual = 610,
	OuterVariableEqualCsf = 611,
	OuterVariableBiggerCsf = 612,
	OuterVariableSmallerCsf = 613,
	OuterVariableNotEqualCsf = 614,
	BuildingAttemptUp = 615,
	AreaHasBuilding=700,
	_DummyMaximum,
};

class TEventExt
{
public:
	using base_type = TEventClass;

	static constexpr DWORD Canary = 0x91919191;

	class ExtData final : public Extension<TEventClass>
	{
	public:
		ExtData(TEventClass* const OwnerObject) : Extension<TEventClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static bool Execute(TEventClass* pThis, int iEvent, HouseClass* pHouse, ObjectClass* pObject,
					CDTimerClass* pTimer, bool* isPersitant, TechnoClass* pSource, bool& bHandled);

	template<bool IsGlobal, typename _Pr>
	static bool VariableCheck(TEventClass* pThis);
	template<bool IsSrcGlobal, bool IsGlobal, typename _Pr>
	static bool VariableCheckBinary(TEventClass* pThis);

	static bool HouseOwnsTechnoTypeTEvent(TEventClass* pThis);
	static bool HouseDoesntOwnTechnoTypeTEvent(TEventClass* pThis);
	static bool SuperWeaponTimerUp(TEventClass* pThis, HouseClass* pHouse);
	static bool OwnerApproachWaypoint(TEventClass* pThis, HouseClass* pHouse);
	static bool ContainsShroud(TEventClass* pThis, ObjectClass* pObject);
	static bool Cheating(TEventClass* pThis);
	static bool OuterVariableEqual(TEventClass* pThis);
	static bool OuterVariableBigger(TEventClass* pThis);
	static bool OuterVariableSmaller(TEventClass* pThis);
	static bool OuterVariableNotEqual(TEventClass* pThis);
	static bool OuterVariableEqualCsf(TEventClass* pThis);
	static bool OuterVariableBiggerCsf(TEventClass* pThis);
	static bool OuterVariableSmallerCsf(TEventClass* pThis);
	static bool OuterVariableNotEqualCsf(TEventClass* pThis);
	static bool BuildingAttemptUp(TEventClass* pThis, HouseClass* pHouse);
	static bool AreaHasBuilding(TEventClass* pThis, HouseClass* pHouse);
	class ExtContainer final : public Container<TEventExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};

#include "Body.h"

#include <BitFont.h>

#include <Utilities/EnumFunctions.h>

BuildingExt::ExtContainer BuildingExt::ExtMap;

void BuildingExt::ExtData::DisplayIncomeString()
{
	if (this->AccumulatedIncome && Unsorted::CurrentFrame % 15 == 0)
	{
		if ((RulesExt::Global()->DisplayIncome_AllowAI || this->OwnerObject()->Owner->IsControlledByHuman())
			&& this->TypeExtData->DisplayIncome.Get(RulesExt::Global()->DisplayIncome))
		{
			FlyingStrings::AddMoneyString(
				this->AccumulatedIncome,
				this->OwnerObject()->Owner,
				this->TypeExtData->DisplayIncome_Houses.Get(RulesExt::Global()->DisplayIncome_Houses.Get()),
				this->OwnerObject()->GetRenderCoords(),
				this->TypeExtData->DisplayIncome_Offset
			);
		}
		this->AccumulatedIncome = 0;
	}
}

bool BuildingExt::ExtData::HasSuperWeapon(const int index, const bool withUpgrades) const
{
	const auto pThis = this->OwnerObject();
	const auto pExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	const auto count = pExt->GetSuperWeaponCount();
	for (auto i = 0; i < count; ++i)
	{
		const auto idxSW = pExt->GetSuperWeaponIndex(i, pThis->Owner);

		if (idxSW == index)
			return true;
	}

	if (withUpgrades)
	{
		for (auto const& pUpgrade : pThis->Upgrades)
		{
			if (const auto pUpgradeExt = BuildingTypeExt::ExtMap.Find(pUpgrade))
			{
				const auto countUpgrade = pUpgradeExt->GetSuperWeaponCount();
				for (auto i = 0; i < countUpgrade; ++i)
				{
					const auto idxSW = pUpgradeExt->GetSuperWeaponIndex(i, pThis->Owner);

					if (idxSW == index)
						return true;
				}
			}
		}
	}

	return false;
}

void BuildingExt::StoreTiberium(BuildingClass* pThis, float amount, int idxTiberiumType, int idxStorageTiberiumType)
{
	auto const pDepositableTiberium = TiberiumClass::Array->GetItem(idxStorageTiberiumType);
	float depositableTiberiumAmount = 0.0f; // Number of 'bails' that will be stored.
	auto const pTiberium = TiberiumClass::Array->GetItem(idxTiberiumType);

	if (amount > 0.0)
	{
		if (auto pBuildingType = pThis->Type)
		{
			if (auto const pExt = BuildingTypeExt::ExtMap.Find(pBuildingType))
			{
				if (pExt->Refinery_UseStorage)
				{
					// Store Tiberium in structures
					depositableTiberiumAmount = (amount * pTiberium->Value) / pDepositableTiberium->Value;
					pThis->Owner->GiveTiberium(depositableTiberiumAmount, idxStorageTiberiumType);
				}
			}
		}
	}
}

void BuildingExt::ExtData::UpdatePrimaryFactoryAI()
{
	auto pOwner = this->OwnerObject()->Owner;

	if (!pOwner || pOwner->ProducingAircraftTypeIndex < 0)
		return;

	AircraftTypeClass* pAircraft = AircraftTypeClass::Array->GetItem(pOwner->ProducingAircraftTypeIndex);
	FactoryClass* currFactory = pOwner->GetFactoryProducing(pAircraft);
	std::vector<BuildingClass*> airFactoryBuilding;
	BuildingClass* newBuilding = nullptr;

	// Update what is the current air factory for future comparisons
	if (this->CurrentAirFactory)
	{
		int nDocks = 0;
		if (this->CurrentAirFactory->Type)
			nDocks = this->CurrentAirFactory->Type->NumberOfDocks;

		int nOccupiedDocks = BuildingExt::CountOccupiedDocks(this->CurrentAirFactory);

		if (nOccupiedDocks < nDocks)
			currFactory = this->CurrentAirFactory->Factory;
		else
			this->CurrentAirFactory = nullptr;
	}

	// Obtain a list of air factories for optimizing the comparisons
	for (auto pBuilding : pOwner->Buildings)
	{
		if (pBuilding->Type->Factory == AbstractType::AircraftType)
		{
			if (!currFactory && pBuilding->Factory)
				currFactory = pBuilding->Factory;

			airFactoryBuilding.emplace_back(pBuilding);
		}
	}

	if (this->CurrentAirFactory)
	{
		for (auto pBuilding : airFactoryBuilding)
		{
			if (pBuilding == this->CurrentAirFactory)
			{
				this->CurrentAirFactory->Factory = currFactory;
				this->CurrentAirFactory->IsPrimaryFactory = true;
			}
			else
			{
				pBuilding->IsPrimaryFactory = false;

				if (pBuilding->Factory)
					pBuilding->Factory->AbandonProduction();
			}
		}

		return;
	}

	if (!currFactory)
		return;

	for (auto pBuilding : airFactoryBuilding)
	{
		int nDocks = pBuilding->Type->NumberOfDocks;
		int nOccupiedDocks = BuildingExt::CountOccupiedDocks(pBuilding);

		if (nOccupiedDocks < nDocks)
		{
			if (!newBuilding)
			{
				newBuilding = pBuilding;
				newBuilding->Factory = currFactory;
				newBuilding->IsPrimaryFactory = true;
				this->CurrentAirFactory = newBuilding;

				continue;
			}
		}

		pBuilding->IsPrimaryFactory = false;

		if (pBuilding->Factory)
			pBuilding->Factory->AbandonProduction();
	}

	return;
}

int BuildingExt::CountOccupiedDocks(BuildingClass* pBuilding)
{
	if (!pBuilding)
		return 0;

	int nOccupiedDocks = 0;

	if (pBuilding->RadioLinks.IsAllocated)
	{
		for (auto i = 0; i < pBuilding->RadioLinks.Capacity; ++i)
		{
			if (auto const pLink = pBuilding->GetNthLink(i))
				nOccupiedDocks++;
		}
	}

	return nOccupiedDocks;
}

bool BuildingExt::HasFreeDocks(BuildingClass* pBuilding)
{
	if (pBuilding->Type->Factory == AbstractType::AircraftType)
	{
		int nDocks = pBuilding->Type->NumberOfDocks;
		int nOccupiedDocks = BuildingExt::CountOccupiedDocks(pBuilding);

		if (nOccupiedDocks < nDocks)
			return true;
		else
			return false;
	}

	return false;
}

bool BuildingExt::CanGrindTechno(BuildingClass* pBuilding, TechnoClass* pTechno)
{
	if (!pBuilding->Type->Grinding || (pTechno->WhatAmI() != AbstractType::Infantry && pTechno->WhatAmI() != AbstractType::Unit))
		return false;

	if ((pBuilding->Type->InfantryAbsorb || pBuilding->Type->UnitAbsorb) &&
		(pTechno->WhatAmI() == AbstractType::Infantry && !pBuilding->Type->InfantryAbsorb ||
			pTechno->WhatAmI() == AbstractType::Unit && !pBuilding->Type->UnitAbsorb))
	{
		return false;
	}

	if (const auto pExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type))
	{
		if (pBuilding->Owner == pTechno->Owner && !pExt->Grinding_AllowOwner)
			return false;

		if (pBuilding->Owner != pTechno->Owner && pBuilding->Owner->IsAlliedWith(pTechno) && !pExt->Grinding_AllowAllies)
			return false;

		if (pExt->Grinding_AllowTypes.size() > 0 && !pExt->Grinding_AllowTypes.Contains(pTechno->GetTechnoType()))
			return false;

		if (pExt->Grinding_DisallowTypes.size() > 0 && pExt->Grinding_DisallowTypes.Contains(pTechno->GetTechnoType()))
			return false;
	}

	return true;
}

bool BuildingExt::DoGrindingExtras(BuildingClass* pBuilding, TechnoClass* pTechno, int refund)
{
	if (auto const pExt = BuildingExt::ExtMap.Find(pBuilding))
	{
		auto const pTypeExt = pExt->TypeExtData;

		pExt->AccumulatedIncome += refund;

		if (pTypeExt->Grinding_Weapon.isset()
			&& Unsorted::CurrentFrame >= pExt->GrindingWeapon_LastFiredFrame + pTypeExt->Grinding_Weapon.Get()->ROF)
		{
			TechnoExt::FireWeaponAtSelf(pBuilding, pTypeExt->Grinding_Weapon.Get());
			pExt->GrindingWeapon_LastFiredFrame = Unsorted::CurrentFrame;
		}

		if (pTypeExt->Grinding_Sound.isset())
		{
			VocClass::PlayAt(pTypeExt->Grinding_Sound.Get(), pTechno->GetCoords());
			return true;
		}
	}

	return false;
}

// Building only or allow units too?
void BuildingExt::ExtData::ApplyPoweredKillSpawns()
{
	auto const pThis = this->OwnerObject();
	auto const pTypeExt = this->TypeExtData;

	if (pTypeExt->Powered_KillSpawns && pThis->Type->Powered && !pThis->IsPowerOnline())
	{
		if (auto pManager = pThis->SpawnManager)
		{
			pManager->ResetTarget();
			for (auto pItem : pManager->SpawnedNodes)
			{
				if (pItem->Status == SpawnNodeStatus::Attacking || pItem->Status == SpawnNodeStatus::Returning)
				{
					pItem->Unit->ReceiveDamage(&pItem->Unit->Health, 0,
						RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
				}
			}
		}
	}
}

bool BuildingExt::HandleInfiltrate(BuildingClass* pBuilding, HouseClass* pInfiltratorHouse)
{
	BuildingTypeExt::ExtData* pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

	if (!pTypeExt->SpyEffect_Custom)
		return false;

	auto pVictimHouse = pBuilding->Owner;
	if (pInfiltratorHouse != pVictimHouse)
	{
		// I assume you were not launching for real, Morton

		auto launchTheSWHere = [pBuilding](SuperClass* const pSuper, HouseClass* const pHouse)
		{
			int oldstart = pSuper->RechargeTimer.StartTime;
			int oldleft = pSuper->RechargeTimer.TimeLeft;
			pSuper->SetReadiness(true);
			pSuper->Launch(CellClass::Coord2Cell(pBuilding->Location), pHouse->IsCurrentPlayer());
			pSuper->Reset();
			pSuper->RechargeTimer.StartTime = oldstart;
			pSuper->RechargeTimer.TimeLeft = oldleft;
		};

		if (pTypeExt->SpyEffect_VictimSuperWeapon.isset())
		{
			if (const auto pSuper = pVictimHouse->Supers.GetItem(pTypeExt->SpyEffect_VictimSuperWeapon.Get()))
				launchTheSWHere(pSuper, pVictimHouse);
		}

		if (pTypeExt->SpyEffect_InfiltratorSuperWeapon.isset())
		{
			if (const auto pSuper = pInfiltratorHouse->Supers.GetItem(pTypeExt->SpyEffect_InfiltratorSuperWeapon.Get()))
				launchTheSWHere(pSuper, pInfiltratorHouse);
		}
	}

	return true;
}

// =============================
// load / save

template <typename T>
void BuildingExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->TypeExtData)
		.Process(this->TechnoExtData)
		.Process(this->DeployedTechno)
		.Process(this->IsCreatedFromMapFile)
		.Process(this->LimboID)
		.Process(this->GrindingWeapon_LastFiredFrame)
		.Process(this->CurrentAirFactory)
		.Process(this->AccumulatedIncome)
		.Process(this->OwnerObject()->LightSource)
		.Process(this->CurrentLaserWeaponIndex)
		.Process(this->BaseNodeClass)
		;
}

void BuildingExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<BuildingClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BuildingExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<BuildingClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool BuildingExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool BuildingExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

BuildingExt::ExtContainer::ExtContainer() : Container("BuildingClass") { }

BuildingExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x43BCBD, BuildingClass_CTOR, 0x6)
{
	GET(BuildingClass*, pItem, ESI);

	auto const pExt = BuildingExt::ExtMap.TryAllocate(pItem);

	if (pExt)
	{
		pExt->TypeExtData = BuildingTypeExt::ExtMap.Find(pItem->Type);
		pExt->TechnoExtData = TechnoExt::ExtMap.Find(pItem);
	}

	return 0;
}

DEFINE_HOOK(0x43C022, BuildingClass_DTOR, 0x6)
{
	GET(BuildingClass*, pItem, ESI);

	BuildingExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x454190, BuildingClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x453E20, BuildingClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BuildingClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x45417E, BuildingClass_Load_Suffix, 0x5)
{
	BuildingExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x454244, BuildingClass_Save_Suffix, 0x7)
{
	BuildingExt::ExtMap.SaveStatic();

	return 0;
}

// Removes setting otherwise unused field (0x6FC) in BuildingClass when building has airstrike applied on it so that it can safely be used to store BuildingExt pointer.
DEFINE_JUMP(LJMP, 0x41D9FB, 0x41DA05);

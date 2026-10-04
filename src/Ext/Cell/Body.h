#pragma once
#include <CellClass.h>
#include <TechnoClass.h>

#include <vector>

#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>

class CellExt
{
public:
	using base_type = CellClass;

	static constexpr DWORD Canary = 0x13371337;
	static constexpr size_t ExtPointerOffset = 0x144;

	class ExtData final : public Extension<CellClass>
	{
	public:
		std::vector<TechnoClass*> AirUnits;
		ExtData(CellClass* OwnerObject) : Extension<CellClass>(OwnerObject)
			, AirUnits {}
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool removed) override;

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void AddAirUnit(TechnoClass* pUnit);
		void RemoveAirUnit(TechnoClass* pUnit);

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<CellExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override;
	};

	static std::vector<TechnoClass*>& GetAirUnits(CellClass* pCell);
	static void AddAirUnit(CellClass* pCell, TechnoClass* pUnit);
	static void RemoveAirUnit(CellClass* pCell, TechnoClass* pUnit);

	static void UpdateAirUnit(FootClass* pLinkedTo);
	static void ClearAirUnit(FootClass* pUnit);
	static std::vector<TechnoClass*> GetAllTrackedAirUnits();
	static void PointerGotInvalid(void* ptr, bool removed);
	static ExtContainer ExtMap;
};

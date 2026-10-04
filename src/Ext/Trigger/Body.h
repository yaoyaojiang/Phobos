#pragma once


#include <TriggerClass.h>
#include <Utilities/Container.h>
#include <Utilities/Enum.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
class TriggerExt
{
public:
	using base_type = TriggerClass;

	static constexpr DWORD Canary = 0xAEFA23EA;
	//static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<TriggerClass>
	{
	public:
		Valueable<CellStruct> Cell;
		ValueableVector<TechnoClass*> AttachedTechnos;

		ExtData(TriggerClass* OwnerObject) : Extension<TriggerClass>(OwnerObject)
			, Cell {}
		{ }

		virtual ~ExtData() = default;


		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TriggerExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	HouseClass* GetOwnerHouse(TriggerClass* pTrigger);

};

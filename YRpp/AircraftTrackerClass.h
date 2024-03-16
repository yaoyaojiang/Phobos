#pragma once

#include <Helpers/CompileTime.h>

#include <GeneralStructures.h>
#include <ArrayClasses.h>

class TechnoClass;

// Tracks aerial units via 20x20 vectors spread across the maps for efficient search
class AircraftTrackerClass
{
public:
	static constexpr reference<AircraftTrackerClass, 0x887888u> Instance { };

	TechnoClass* Get() { JMP_THIS(0x4137A0) }

	void Add(TechnoClass* entry) { JMP_THIS(0x4134A0) }
	void Update(TechnoClass* entry, CellStruct oldPos, CellStruct newPos) { JMP_THIS(0x4138C0) }
	void Remove(TechnoClass* entry) { JMP_THIS(0x4135D0) }

	bool Clear() { JMP_THIS(0x413800) }

	bool IsJumpjet(TechnoClass* entry) { JMP_THIS(0x4135A0) }
	int GetVectorIndex(CellStruct pos) { JMP_THIS(0x412AC0) }

	// TODO write other entries

private:
	AircraftTrackerClass() {}

public:
	DynamicVectorClass<TechnoClass*> TrackerVectors[20][20];
	DynamicVectorClass<TechnoClass*> CurrentVector;
};

static_assert(sizeof(AircraftTrackerClass) == 0x2598);

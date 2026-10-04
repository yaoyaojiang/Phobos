#include <JumpjetLocomotionClass.h>
#include <FlyLocomotionClass.h>
#include <FootClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include <ObjectClass.h>

#include "Body.h"

#include <Utilities/Macro.h>

// =============================
// JumpjetLocomotionClass::MovementUpdate
// 出口：FlyLocomotionClass::Process 中 call MovementUpdate 返回后。
// 此时 esi = Process 的 this(FrameLayoutOffset*)，LinkedTo 位于 [esi+8]。
DEFINE_HOOK(0x54aef5, JumpjetLocomotionClass_MovementUpdate_Exit, 0x9)
{
	GET(BYTE*, pThis, ESI);
	CellExt::UpdateAirUnit(*reinterpret_cast<FootClass**>(pThis + 8));
	return 0;
}

// =============================
// FlyLocomotionClass::UpdateLocation
// 出口：FlyLocomotionClass::Process 中 call UpdateLocation 返回后。
// 此时 esi = Process 的 this(FrameLayoutOffset*)，LinkedTo 位于 [esi+8]。
DEFINE_HOOK(0x4ccbc4, FlyLocomotionClass_UpdateLocation_Exit, 0x6)
{
	GET(BYTE*, pThis, ESI);
	CellExt::UpdateAirUnit(*reinterpret_cast<FootClass**>(pThis + 8));
	return 0;
}

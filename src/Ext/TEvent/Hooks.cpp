#include "Body.h"

#include <Helpers\Macro.h>

#include <HouseClass.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <OverlayTypeClass.h>
#include <VocClass.h>
#include <TriggerClass.h>
#include <TriggerTypeClass.h>

#include <Utilities/Macro.h>

DEFINE_HOOK(0x71E940, TEventClass_Execute, 0x5)
{
	GET(TEventClass*, pThis, ECX);
	GET_STACK(int, iEvent, 0x4); // now trigger what?
	GET_STACK(HouseClass*, pHouse, 0x8);
	GET_STACK(ObjectClass*, pObject, 0xC);
	GET_STACK(CDTimerClass*, pTimer, 0x10);
	GET_STACK(bool*, isPersitant, 0x14);
	GET_STACK(TechnoClass*, pSource, 0x18);

	bool handled;

	R->AL(TEventExt::Execute(pThis, iEvent, pHouse, pObject, pTimer, isPersitant, pSource, handled));

	return handled ? 0x71EA2D : 0;
}

DEFINE_HOOK(0x7271F9, TEventClass_GetFlags, 0x5)
{
	GET(int, eAttach, EAX);
	GET(TEventClass*, pThis, ESI);

	int nEvent = static_cast<int>(pThis->EventKind);
	if (nEvent >= PhobosTriggerEvent::LocalVariableGreaterThan && nEvent < PhobosTriggerEvent::_DummyMaximum)
		eAttach |= 0x10; // LOGIC

	R->EAX(eAttach);

	return 0;
}

DEFINE_HOOK(0x71F3FE, TEventClass_BuildINIEntry, 0x5)
{
	GET(int, eNeedType, EAX);
	GET(TEventClass*, pThis, ECX);

	int nEvent = static_cast<int>(pThis->EventKind);
	if (nEvent >= PhobosTriggerEvent::LocalVariableGreaterThan && nEvent < PhobosTriggerEvent::_DummyMaximum)
		eNeedType = 43;

	R->EAX(eNeedType);

	return 0;
}

DEFINE_HOOK(0x726577, TEventClass_Persistable, 0x7)
{
	GET(TEventClass*, pThis, EDI);

	int nEvent = static_cast<int>(pThis->EventKind);
	if (nEvent >= PhobosTriggerEvent::LocalVariableGreaterThan && nEvent < PhobosTriggerEvent::_DummyMaximum)
		R->AL(true);
	else
		R->AL(pThis->GetStateB());

	return 0x72657E;
}

/*
* 用于修复触发事件38-43响应一次后以后会一直响应的问题，这个钩子开启后必须严格再次触发（当然其他事件也可以加进去）
	Re-arm health threshold events when a trigger is enabled by the
	EnableTrigger action (53).

	Base game bug: TriggerClass::OccuredEvents latches the event slot once a
	health threshold event (38-43) has fired on a Persistent tag. The
	EnableTrigger action (sub_7268F0) only sets Enabled and resets timers,
	it never clears the latch bits. Since LogicClass::Update bubbles
	ElapsedTime (13) to every tag each frame, RegisterEvent (sub_7264C0)
	short-circuits on the latched bit, so a re-enabled trigger re-fires
	immediately on the next frame, even if the attached object was repaired
	and never crossed the threshold again.

	Fix: when the EnableTrigger action enables a trigger, clear the latch
	bits of all health threshold events, so the trigger has to wait for an
	actual new threshold crossing. Latch bits of other event kinds are left
	untouched to preserve multi-event (AND) semantics.

	sub_7268F0 is:
		mov byte ptr [ecx+44h], 1   ; this->Enabled = true (4 bytes)
		jmp sub_726400              ; tail-jump into ResetTimers (5 bytes)
	The 0x9 hook size steals both instructions, including the relative
	jmp. A relative branch re-executed from syringe's copy of the stolen
	bytes lands at a wrong address (its displacement is location
	dependent), so this hook must never return 0. It reproduces the
	mov itself and jumps straight into ResetTimers instead. ECX still
	holds pThis when syringe restores the registers.
*/
DEFINE_HOOK(0x7268F0, TriggerClass_EnableAction_ReArmHealthEvents, 0x9)
{
	GET(TriggerClass*, pThis, ECX);

	// reproduce mov byte ptr [ecx+44h], 1
	pThis->Enabled = true;

	if (auto pType = pThis->Type)
	{
		// sub_7264C0 indexes OccuredEvents by the slot position of each
		// event in the type's event chain, mirror that walk here
		int slot = 0;

		for (auto pEvent = pType->FirstEvent; pEvent && slot < 32; pEvent = pEvent->NextEvent, ++slot)
		{
			switch (pEvent->EventKind)
			{
			case TriggerEvent::FirstDamaged_combatonly:   // 38
			case TriggerEvent::HalfHealth_combatonly:     // 39
			case TriggerEvent::QuarterHealth_combatonly:  // 40
			case TriggerEvent::FirstDamaged_anysource:    // 41
			case TriggerEvent::HalfHealth_anysource:      // 42
			case TriggerEvent::QuarterHealth_anysource:   // 43
				pThis->MarkEventAsNotOccured(slot);
				break;
			default:
				break;
			}
		}
	}

	// tail-jump into TriggerClass::ResetTimers like the original code does;
	// never return 0 here, the stolen bytes contain a relative jmp
	return 0x726400;
}

// 
// BaseAbility.cpp
// 
// Implementation of the 'BaseAbility' class.
// 
// ----------------------------------------  x  ---------------------------------------- 
// 
// © 2026 CylindriKill. All rights reserved.
// 

#include "BaseAbility.h"

#include "BaseCharacter.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

// ------------------------------------------------------------------
// Constructor & Destructor
// ------------------------------------------------------------------
UBaseAbility::UBaseAbility()
{
	CooldownTime = 1.0f;
}

// ------------------------------------------------------------------
// Exposed Methods
// ------------------------------------------------------------------
bool UBaseAbility::Activate()
{
	if (IsOnCooldown())
		return false;
	
	GetWorld()->GetTimerManager().SetTimer(
	   CooldownTimerHandle,
	   FTimerDelegate(),
	   CooldownTime,
	   false
	);
	
	return true;
}

bool UBaseAbility::IsOnCooldown() const
{
	return GetWorld()->GetTimerManager().IsTimerActive(CooldownTimerHandle);
}

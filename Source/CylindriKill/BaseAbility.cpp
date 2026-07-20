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

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ------------------------------------------------------------------
// Constructor & Destructor
// ------------------------------------------------------------------
UBaseAbility::UBaseAbility()
{
	bCanTrigger  = true;
	CooldownTime = 1.0f;
}

// ------------------------------------------------------------------
// Overridden Methods
// ------------------------------------------------------------------
void UBaseAbility::BeginPlay()
{
	Super::BeginPlay();
	
	Parent = Cast<ABaseCharacter>(GetOwner());
}

bool UBaseAbility::IsActive() const
{
	return false;
}

bool UBaseAbility::Trigger()
{
	if (!bCanTrigger)
		return false;
	
	bCanTrigger = false;

	GetWorld()->GetTimerManager().SetTimer(
	   CooldownTimerHandle,
	   FTimerDelegate::CreateLambda([this]()
	   {
		   bCanTrigger = true;
	   }),
	   CooldownTime,
	   false
	);
	
	return true;
}

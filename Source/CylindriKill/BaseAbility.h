// 
// BaseAbility.h
// 
// Parent class for all (both enemy & player) abilities.
// 
// ----------------------------------------  x  ---------------------------------------- 
// 
// © 2026 CylindriKill. All rights reserved.
// 

#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "UObject/Object.h"
#include "BaseAbility.generated.h"

UCLASS(EditInlineNew, Blueprintable, BlueprintType)
class CYLINDRIKILL_API UBaseAbility : public UObject
{
	GENERATED_BODY()
    
	// ------------------------------------------------------------------
	// Internal Variables
	// ------------------------------------------------------------------
	FTimerHandle CooldownTimerHandle;
    
protected:
    
	// ------------------------------------------------------------------
	// Blueprint Variables
	// ------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay|Ability")
	float CooldownTime;
    
public:
    
	// ------------------------------------------------------------------
	// Constructor & Destructor
	// ------------------------------------------------------------------
	UBaseAbility();
    
	// ------------------------------------------------------------------
	// Exposed Methods
	// ------------------------------------------------------------------
	virtual bool Activate();
	virtual bool IsOnCooldown() const;
};
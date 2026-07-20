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

#include "BaseCharacter.h"
#include "BaseAbility.generated.h"

UCLASS(Blueprintable, BlueprintType, DefaultToInstanced, EditInlineNew)
class CYLINDRIKILL_API UBaseAbility : public UActorComponent
{
	GENERATED_BODY()
	
protected:
	
	// ------------------------------------------------------------------
	// Internal Variables
	// ------------------------------------------------------------------
	bool bCanTrigger;
	
	UPROPERTY() TObjectPtr<ABaseCharacter> Parent;
	UPROPERTY() FTimerHandle CooldownTimerHandle;
	
	// ------------------------------------------------------------------
	// Blueprint Variables
	// ------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"))
	float CooldownTime;
	
public:
	
	// ------------------------------------------------------------------
	// Constructor & Destructor
	// ------------------------------------------------------------------
	UBaseAbility();
	
	// ------------------------------------------------------------------
	// Exposed Methods
	// ------------------------------------------------------------------
	virtual void BeginPlay() override;
	virtual bool IsActive() const;
	virtual bool Trigger();
};

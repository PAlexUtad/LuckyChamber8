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
	bool         bCanTrigger;
	FTimerHandle CooldownTimer;

	TWeakObjectPtr<ABaseCharacter> Parent;
	
	// ------------------------------------------------------------------
	// Blueprint Variables
	// ------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Dash", Meta = (AllowPrivateAccess = "true"))
	float CooldownDuration;
	
public:
	
	// ------------------------------------------------------------------
	// Constructor & Destructor
	// ------------------------------------------------------------------
	UBaseAbility();
	
	// ------------------------------------------------------------------
	// Overridden Methods
	// ------------------------------------------------------------------
	virtual void BeginAbility();
	virtual void TickAbility(float);
	virtual void EndAbility();
};

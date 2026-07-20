
#include "BaseAbility.h"

#include "TimerManager.h"
#include "Engine/World.h"

UBaseAbility::UBaseAbility()
{
	Parent = Cast<ABaseCharacter>(this->GetOwner());
	
	// WARNING: This will asset that 'Parent' is a ABaseCharacter, crashing if it is not.
	// check(Parent);
}

void UBaseAbility::BeginAbility()
{
}

void UBaseAbility::TickAbility(float)
{
}

void UBaseAbility::EndAbility()
{
	bCanTrigger = false;

	GetWorld()->GetTimerManager().SetTimer(
	   CooldownTimer,
	   FTimerDelegate::CreateLambda([this]()
	   {
		   bCanTrigger = true;
	   }),
	   CooldownDuration,
	   false
	);
}

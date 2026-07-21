// 
// BaseCharacter.cpp
// 
// Implementation of the 'BaseCharacter' class. 
// 
// ----------------------------------------  x  ---------------------------------------- 
// 
// © 2026 CylindriKill. All rights reserved.
// 

#include "BaseCharacter.h"

#include "BaseAbility.h"
#include "Component/HealthComponent.h"
#include "Engine/Engine.h"
#include "PaperSpriteComponent.h"

// ------------------------------------------------------------------
// Constructor & Destructor
// ------------------------------------------------------------------
ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
	SpriteComponent->SetupAttachment(RootComponent);
	SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->MaxHealth = 1.f;
}

// ------------------------------------------------------------------
// Overridden Methods
// ------------------------------------------------------------------
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	HealthComponent->OnDeath.AddDynamic(this, &ABaseCharacter::Die);
}

void ABaseCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// ------------------------------------------------------------------
// Internal Methods
// ------------------------------------------------------------------
void ABaseCharacter::Die(AActor* Aggressor)
{
	// TODO: Take death animation into account with a delay before destroying the character.
	Destroy();
}

void ABaseCharacter::ActivateAbility(const TSubclassOf<UBaseAbility> AbilityClass)
{
	for (const TObjectPtr Ability : Abilities)
	{
		if (Ability->GetClass() == AbilityClass)
		{
			Ability->Activate();
			return;
		}
	}
}

UBaseAbility* ABaseCharacter::FindAbility(const TSubclassOf<UBaseAbility> AbilityClass)
{
	for (TObjectPtr Ability : Abilities)
	{
		if (Ability->GetClass() == AbilityClass)
			return Ability;
	}
	
	return nullptr;
}

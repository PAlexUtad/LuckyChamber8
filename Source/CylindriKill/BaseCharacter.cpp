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

#include "Component/HealthComponent.h"
#include "Engine/Engine.h"
#include "PaperSpriteComponent.h"

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
	
	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &ABaseCharacter::Die);
	}
}

void ABaseCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// ------------------------------------------------------------------
// Internal Methods
// ------------------------------------------------------------------
void ABaseCharacter::Spawn()
{
	
}

void ABaseCharacter::Die(AActor* Aggressor)
{
	// TODO: Take death animation into account with a delay before destroying the character.
	Destroy();
}

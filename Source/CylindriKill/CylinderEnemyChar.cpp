// Fill out your copyright notice in the Description page of Project Settings.

#include "CylinderEnemyChar.h"

#include "BaseProjectile.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Plugins/2D/Paper2D/Source/Paper2D/Classes/PaperSpriteComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ACylinderEnemyChar::ACylinderEnemyChar()
{
	PrimaryActorTick.bCanEverTick = true;

	// Configure character movement for navmesh walking
	// Ensure the capsule component blocks Visibility so hitscan traces catch it
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
	
		Capsule->InitCapsuleSize(34.f, 90.f);
		Capsule->SetCollisionObjectType(ECC_Pawn);
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->SetCollisionResponseToAllChannels(ECR_Block);
		Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); // <--- Add this!
	}
	
	// Disable default controller rotation yaw so we can manually rotate just the sprite or actor cleanly to face the player
	bUseControllerRotationYaw = false;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = false; // We override rotation to face the player instead of moving direction
	}

	// Setup Paper Sprite Component attached to root capsule
	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
	SpriteComponent->SetupAttachment(RootComponent);
	SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACylinderEnemyChar::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld() && ProjectileClass)
	{
		GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ACylinderEnemyChar::FireAtPlayer, ShootInterval, true);
	}
}

void ACylinderEnemyChar::FireAtPlayer()
{
	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn || !ProjectileClass) return;

	const FVector SpawnLocation = GetActorLocation() + (GetActorForwardVector() * 50.f) + FVector(0.f, 0.f, 30.f);
	const FRotator SpawnRotation = (PlayerPawn->GetActorLocation() - SpawnLocation).Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	GetWorld()->SpawnActor<ABaseProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
}

void ACylinderEnemyChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bFacePlayer)
	{
		RotateToFacePlayer(DeltaTime);
	}
}

void ACylinderEnemyChar::RotateToFacePlayer(float DeltaTime)
{
	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		return;
	}

	const FVector StartLocation = GetActorLocation();
	const FVector TargetLocation = PlayerPawn->GetActorLocation();
	const FVector Direction = (TargetLocation - StartLocation).GetSafeNormal();

	FRotator NewRotation = Direction.Rotation();
	NewRotation.Pitch = 0.f;
	NewRotation.Roll = 0.f;

	SetActorRotation(NewRotation);
}

float ACylinderEnemyChar::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	Health -= ActualDamage;
	if (Health <= 0.f)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Enemy Character Died!"));
		}
		Destroy();
	}

	return ActualDamage;
}
// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseEnemy.h"

#include "BaseProjectile.h"
#include "CylindriKill/Component/HealthComponent.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Plugins/2D/Paper2D/Source/Paper2D/Classes/PaperSpriteComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Runtime/AIModule/Classes/AIController.h"

ABaseEnemy::ABaseEnemy()
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

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->MaxHealth = 1.f; // preserves the old one-shot-kill feel; raise on a per-enemy Blueprint for tankier types

	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &ABaseEnemy::HandleDeath);
	}

	if (GetWorld() && ProjectileClass)
	{
		GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ABaseEnemy::FireAtPlayer, ShootInterval, true);
	}
}

void ABaseEnemy::FireAtPlayer()
{
	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn || !ProjectileClass) return;

	if (!IsPlayerInRange()) return; // don't fire while dormant, even if a timer already queued this call

	const FVector SpawnLocation = GetActorLocation() + (GetActorForwardVector() * 50.f) + FVector(0.f, 0.f, 30.f);
	const FRotator SpawnRotation = (PlayerPawn->GetActorLocation() - SpawnLocation).Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	GetWorld()->SpawnActor<ABaseProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
}

bool ABaseEnemy::IsPlayerInRange() const
{
	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		return false;
	}

	const float DistSq = FVector::DistSquared(GetActorLocation(), PlayerPawn->GetActorLocation());
	return DistSq <= FMath::Square(MaxActivationRange);
}
void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsPlayerInRange())
	{
		return; // dormant - no facing, no movement, no shooting logic runs at all
	}

	if (bFacePlayer)
	{
		RotateToFacePlayer(DeltaTime);
	}
	UpdateMovement(DeltaTime);
}

void ABaseEnemy::RotateToFacePlayer(float DeltaTime)
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

float ABaseEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (HealthComponent)
	{
		HealthComponent->ApplyDamage(ActualDamage > 0.f ? ActualDamage : DamageAmount, DamageCauser, EventInstigator);
	}

	return ActualDamage;
}
void ABaseEnemy::HandleDeath(AActor* DamageCauser)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Enemy Character Died!"));
	}
	Destroy();
}
void ABaseEnemy::MoveToLocationViaNav(const FVector& TargetLocation, float AcceptanceRadius)
{
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->MoveToLocation(
			TargetLocation, AcceptanceRadius, /*bStopOnOverlap*/ true, /*bUsePathfinding*/ true,
			/*bProjectDestinationToNavigation*/ true, /*bCanStrafe*/ true, nullptr, /*bAllowPartialPath*/ true);
	}
}

void ABaseEnemy::MoveToActorViaNav(AActor* TargetActor, float AcceptanceRadius)
{
	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Red, TEXT("No AIController!"));
		return;
	}

	if (!TargetActor) {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Red, TEXT("No TARGET!"));
		return;
	}

	const EPathFollowingRequestResult::Type Result = AIController->MoveToActor(TargetActor, AcceptanceRadius, true, true, true);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow,
			FString::Printf(TEXT("MoveToActor result: %d"), (int32)Result));
	}
}

void ABaseEnemy::StopNavMovement()
{
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
	}
}
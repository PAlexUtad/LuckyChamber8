#include "CylinderEnemyChar_Charger.h"

#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

ACylinderEnemyChar_Charger::ACylinderEnemyChar_Charger()
{
	// Needs a ProjectileClass set on the Blueprint now — it shoots, it doesn't melee
}

void ACylinderEnemyChar_Charger::BeginPlay()
{
	Super::BeginPlay();
	NormalWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	// Base class BeginPlay starts a fire timer automatically if ProjectileClass is set —
	// we don't want it shooting until it's actually closed the distance, so stop it here.
	GetWorldTimerManager().ClearTimer(FireTimerHandle);
}

void ACylinderEnemyChar_Charger::UpdateMovement(float DeltaTime)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Cyan, TEXT("UpdateMovement called"));
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Cyan, TEXT("NOOOOO PlayerPawn"));
		return;
	}

	const float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

	if (bIsShooting)
	{
		// If the player runs far enough away, stop planting and resume the chase
		if (Distance > FireRange * 1.5f)
		{
			bIsShooting = false;
			GetWorldTimerManager().ClearTimer(FireTimerHandle);
		}
		return; // base Tick already calls FireAtPlayer via the timer while bIsShooting is active
	}

	if (bIsCharging)
	{
		MoveToActorViaNav(PlayerPawn, FireRange * 0.9f);

		if (Distance <= FireRange)
		{
			bIsCharging = false;
			bIsShooting = true;
			StopNavMovement();
			GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
			GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ACylinderEnemyChar_Charger::FireAtPlayer, CloseRangeShootInterval, true, 0.f);		}
		return;
	}

	if (TelegraphRemaining > 0.f)
	{
		StopNavMovement(); // root in place during windup — the tell before the charge
		TelegraphRemaining -= DeltaTime;
		if (TelegraphRemaining <= 0.f)
		{
			bIsCharging = true;
			GetCharacterMovement()->MaxWalkSpeed = ChargeSpeed;
		}
		return;
	}

	if (Distance <= TriggerRange)
	{
		TelegraphRemaining = TelegraphTime;
	}
	else
	{
		MoveToActorViaNav(PlayerPawn, TriggerRange * 0.9f);
	}
}
// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseGun.h"

#include "CylinderEnemyChar.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "BaseProjectile.h"
#include "CylinderPlayer.h"

// Sets default values
ABaseGun::ABaseGun()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Root component - no mesh of its own, just an attach point for BodyMesh (and anything subclasses add).
	GunRoot = CreateDefaultSubobject<USceneComponent>(TEXT("GunRoot"));
	RootComponent = GunRoot;

	// Main body mesh, attached to the root. Left mesh-less here on purpose -
	// ABaseGun stays generic; subclasses decide what the body actually looks like.
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ABaseGun::BeginPlay()
{
	Super::BeginPlay();

	CurrentAmmo = MaxAmmo;
}

// Called every frame
void ABaseGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Inside Tick(float DeltaTime):
	CurrentRecoilLocation = FMath::VInterpTo(CurrentRecoilLocation, TargetRecoilLocation, DeltaTime, RecoilInterpSpeed);
	CurrentRecoilRotation = FMath::RInterpTo(CurrentRecoilRotation, TargetRecoilRotation, DeltaTime, RecoilInterpSpeed);

	// Decay the target back to zero organically
	TargetRecoilLocation = FMath::VInterpTo(TargetRecoilLocation, FVector::ZeroVector, DeltaTime, RecoilInterpSpeed * 0.7f);
	TargetRecoilRotation = FMath::RInterpTo(TargetRecoilRotation, FRotator::ZeroRotator, DeltaTime, RecoilInterpSpeed * 0.7f);

	// Apply relative to root/base transform
	if (GunRoot)
	{
		GunRoot->SetRelativeLocation(CurrentRecoilLocation);
		GunRoot->SetRelativeRotation(CurrentRecoilRotation);
	}

	// Smoothly animate the barrel opening and closing
	CurrentBarrelRotation = FMath::RInterpTo(CurrentBarrelRotation, TargetBarrelRotation, DeltaTime, 15.f);
	if (BodyMesh) // Or whichever sub-mesh represents your opening barrel/slide
	{
		BodyMesh->SetRelativeRotation(CurrentBarrelRotation);
	}
}


void ABaseGun::StartParry()
{
    if (!bCanParry || bIsParrying) return;

    bIsParrying = true;
    bCanParry = true; // prevent re-triggering instantly
    TargetBarrelRotation = BarrelOpenRotationOffset; // Trigger open animation

    if (GEngine)
    {
       GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Magenta, TEXT("Parry Active!"));
    }

    // Check for incoming projectiles immediately during parry frame
    PerformParryTrace();

    // End parry window after short duration
    GetWorldTimerManager().SetTimer(ParryTimerHandle, this, &ABaseGun::EndParry, ParryDuration, false);
}

void ABaseGun::StopParry()
{
    // Optional early release logic if needed
}

void ABaseGun::PerformParryTrace()
{
	const APawn* OwningPawn = Cast<APawn>(GetOwner());
	const UCameraComponent* AimCamera = OwningPawn ? OwningPawn->FindComponentByClass<UCameraComponent>() : nullptr;
	if (!AimCamera) return;

	// Increased reach from 80.f to 250.f so fast projectiles aren't skipped
	const FVector TraceStart = AimCamera->GetComponentLocation();
	const FVector TraceEnd = TraceStart + (AimCamera->GetForwardVector() * 250.f); 

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	if (OwningPawn) ActorsToIgnore.Add(const_cast<APawn*>(OwningPawn));

	// Search specifically for WorldDynamic objects (which is what your ABaseProjectile is set to)
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

	FHitResult HitResult;
	const bool bHit = UKismetSystemLibrary::SphereTraceSingleForObjects(
		GetWorld(),
		TraceStart,
		TraceEnd,
		ParrySphereRadius,
		ObjectTypes,
		false,
		ActorsToIgnore,
		bDrawDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		HitResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		1.5f
	);

	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();
		if (ABaseProjectile* Projectile = Cast<ABaseProjectile>(HitActor))
		{
			// Caught the bullet! Destroy the enemy projectile and give feedback
			Projectile->Destroy();
			AddAmmo();
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("PARRY SUCCESSFUL! Bullet Caught!"));
			}
		}
	}
}

void ABaseGun::EndParry()
{
    bIsParrying = false;
    TargetBarrelRotation = FRotator::ZeroRotator; // Close barrel animation

    // Start cooldown
    GetWorldTimerManager().SetTimer(ParryCooldownTimerHandle, this, &ABaseGun::ResetParryCooldown, ParryCooldown, false);
}

void ABaseGun::AddAmmo()
{
	if (CurrentAmmo < 8)
	{
		CurrentAmmo++;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Orange, TEXT("Full Ammo! Making Bullets Stronger"));
	}
}

void ABaseGun::ResetParryCooldown()
{
    bCanParry = true;
}

void ABaseGun::StartFire()
{
	// Default behaviour is semi-automatic: one trace per press.
	// Automatic weapons can override this to kick off a repeating timer instead.
	
	Fire();
}

void ABaseGun::StopFire()
{
	// No-op for semi-auto guns. Automatic-weapon subclasses should clear their
	// repeating fire timer here.
}

void ABaseGun::Fire()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("ABaseGun FIRE!"));
	// -------------------------------------------------------------
	// Cooldown + ammo gates - stops a semi-auto weapon being spammed
	// faster than FireRate, and stops firing once the mag is empty.
	// -------------------------------------------------------------
	if (!bCanFire)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Cannot Fire"));
		return;
	}

	if (CurrentAmmo <= 0)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Orange, TEXT("Click. Out of ammo."));
		}
		return;
	}
	
	// -------------------------------------------------------------
	// Find the camera to aim from. The gun is a child actor attached
	// under the player's camera, so we look the camera up via the
	// owning pawn rather than depending on any specific player class.
	// -------------------------------------------------------------
	const APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (IsValid(OwningPawn))
	{
		Cast<ACylinderPlayer>(OwningPawn)->PlayShootCameraShake();
	}
	const UCameraComponent* AimCamera = OwningPawn ? OwningPawn->FindComponentByClass<UCameraComponent>() : nullptr;
	if (!AimCamera)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("No camera"));
		return;
	}

	const FVector TraceStart = AimCamera->GetComponentLocation();
	const FVector TraceEnd = TraceStart + (AimCamera->GetForwardVector() * TraceRange);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GunFire), /*bTraceComplex=*/true);
	QueryParams.AddIgnoredActor(this);
	if (OwningPawn)
	{
		QueryParams.AddIgnoredActor(OwningPawn);
	}

	FHitResult HitResult;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	if (bDrawDebugTrace)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("bDrawDebugTrace!"));
		if (bHit)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("bHit!"));
			DrawDebugLine(GetWorld(), TraceStart, HitResult.ImpactPoint, FColor::Yellow, false, 1.5f, 0, 1.f);
			DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 8.f, 12, FColor::Red, false, 1.5f);
		}
		else
		{
			DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Blue, false, 1.5f, 0, 1.f);
		}
	}

	if (bHit && GEngine)
	{
		AActor* HitActor = HitResult.GetActor();
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green,
			FString::Printf(TEXT("Hit: %s"), *GetNameSafe(HitActor)));

		// Directly check if the hit actor is our enemy character and delete it instantly
		if (HitActor)
		{
			// If you named your character class ACylinderEnemyChar, cast and destroy:
			if (ACylinderEnemyChar* Enemy = Cast<ACylinderEnemyChar>(HitActor))
			{
				Enemy->Destroy();
			}
		}
	}

	--CurrentAmmo;
	// Inside Fire() when the shot goes through:
	TargetRecoilLocation.X += RecoilKickBack;
	TargetRecoilRotation.Pitch += RecoilKickPitch;

	// Gate the next shot until FireRate seconds have passed.
	bCanFire = false;
	GetWorldTimerManager().SetTimer(FireCooldownTimerHandle, this, &ABaseGun::ResetFireCooldown, FireRate, false);
}

void ABaseGun::ResetFireCooldown()
{
	bCanFire = true;
}

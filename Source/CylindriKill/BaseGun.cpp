// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseGun.h"

#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

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
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green,
			FString::Printf(TEXT("Hit: %s"), *GetNameSafe(HitResult.GetActor())));
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

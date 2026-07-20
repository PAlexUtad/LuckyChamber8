// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseGun.h"

#include "BaseProjectile.h"
#include "Camera/CameraComponent.h"
#include "CylindriKill/Character/PlayerCharacter.h"
#include "CylindriKill/Component/HealthComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
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

void ABaseGun::BeginPlay()
{
	Super::BeginPlay();
    
	// Cache locations/rotations
	CylinderBaseLoc = CylinderMesh ? CylinderMesh->GetRelativeLocation() : FVector::ZeroVector;
	CylinderBaseRotation = CylinderMesh ? CylinderMesh->GetRelativeRotation() : FRotator::ZeroRotator;
	CurrentCylinderSlideLocation = CylinderBaseLoc;
	TargetCylinderSlideLocation = CylinderBaseLoc;

	// Initialize ChamberTiers instead of the old bool array
	ChamberTiers.Init(0, NumCylinderChambers); 
    
	// Start with a full cylinder (Tier 1 bullets)
	for(int32 i = 0; i < ChamberTiers.Num(); i++)
	{
		ChamberTiers[i] = 1;
	}
    
	CurrentAmmo = NumCylinderChambers;
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

	// Smoothly slide the cylinder mesh out (and back) during parry
	CurrentCylinderSlideLocation = FMath::VInterpTo(CurrentCylinderSlideLocation, TargetCylinderSlideLocation, DeltaTime, CylinderSlideInterpSpeed);
	if (CylinderMesh)
	{
		CylinderMesh->SetRelativeLocation(CurrentCylinderSlideLocation);
	}

	// Scroll-driven cylinder rotation: each scroll tick snaps CurrentChamberIndex forward/back by
	// one chamber, TargetCylinderSpinDegrees becomes an exact multiple of (360/NumCylinderChambers),
	// and we smoothly interpolate the visual rotation toward that fixed step - a proper indexed
	// revolver click rather than a freely coasting spin.
	CurrentCylinderSpinDegrees = FMath::FInterpTo(CurrentCylinderSpinDegrees, TargetCylinderSpinDegrees, DeltaTime, CylinderRotationInterpSpeed);

	if (CylinderMesh)
	{
		FRotator SpinOffset = FRotator::ZeroRotator;
		if (bSpinAroundRollAxis)
		{
			SpinOffset.Roll = CurrentCylinderSpinDegrees;
		}
		else
		{
			SpinOffset.Yaw = CurrentCylinderSpinDegrees;
		}
		CylinderMesh->SetRelativeRotation(CylinderBaseRotation + SpinOffset);
	}

	if (bIsParrying)
	{
		// Check for incoming projectiles immediately during parry frame
		PerformParryTrace();
	}
}


void ABaseGun::StartParry()
{
	if (!bCanParry || bIsParrying) return;

	bIsParrying = true;
	bCanParry = true; // prevent re-triggering instantly
	TargetBarrelRotation = BarrelOpenRotationOffset; // Trigger open animation
	TargetCylinderSlideLocation = CylinderSlideOffset; // Trigger cylinder slide-out

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Magenta, TEXT("Parry Active!"));
	}
	// End parry window after short duration
	GetWorldTimerManager().SetTimer(ParryTimerHandle, this, &ABaseGun::EndParry, ParryDuration, false);
}

void ABaseGun::StopParry()
{
    // Optional early release logic if needed
}
void ABaseGun::AddCylinderScrollInput(float ScrollValue)
{
	if (NumCylinderChambers <= 0) return;

	const float StepDegrees = 360.f / static_cast<float>(NumCylinderChambers); // 45° for 8 chambers

	// One scroll tick = one chamber step, in whichever direction the wheel moved.
	const int32 Direction = (ScrollValue > 0.f) ? 1 : (ScrollValue < 0.f ? -1 : 0);
	if (Direction == 0) return;

	CurrentChamberIndex = (CurrentChamberIndex + Direction + NumCylinderChambers) % NumCylinderChambers;

	// Accumulate rather than wrap the *visual* target, so the interpolation always spins the
	// short way forward/backward instead of snapping backward through 315° when wrapping past 0.
	TargetCylinderSpinDegrees += Direction * StepDegrees;
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
	TargetCylinderSlideLocation = CylinderBaseLoc; // Slide cylinder back in

	// Start cooldown
	GetWorldTimerManager().SetTimer(ParryCooldownTimerHandle, this, &ABaseGun::ResetParryCooldown, ParryCooldown, false);
}

void ABaseGun::AddAmmo()
{
	// 1. First pass: Look for empty chambers (Tier 0) and fill them with Tier 1
	for (int32 i = 0; i < ChamberTiers.Num(); ++i)
	{
		if (ChamberTiers[i] == 0)
		{
			ChamberTiers[i] = 1;
			CurrentAmmo = FMath::Min(CurrentAmmo + 1, ChamberTiers.Num());
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, TEXT("Chamber loaded with Tier 1!"));
			return; 
		}
	}

	// 2. Smooth Upgrade Pass
	// We want to find the lowest tier that is currently less than 4, 
	// and upgrade only the first instance of it.
	for (int32 TargetTier = 1; TargetTier < 4; ++TargetTier)
	{
		for (int32 i = 0; i < ChamberTiers.Num(); ++i)
		{
			if (ChamberTiers[i] == TargetTier)
			{
				ChamberTiers[i]++; // Upgrade this one
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, 
					FString::Printf(TEXT("Upgraded Chamber %d to Tier %d"), i, ChamberTiers[i]));
				return; // Exit after one upgrade to keep it balanced
			}
		}
	}

	// 3. If we got here, all bullets are Tier 4
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Orange, TEXT("All chambers at Max Tier!"));
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

    // 1. Cooldown gate
    if (!bCanFire) return;

    // 2. Ammo gate
    if (!ChamberTiers.IsValidIndex(CurrentChamberIndex) || ChamberTiers[CurrentChamberIndex] <= 0)
    {
       if (GEngine)
       {
          GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Orange,
             FString::Printf(TEXT("Click. Chamber %d is empty."), CurrentChamberIndex));
       }
       return;
    }

    // 3. Determine damage and SHOW TIER
    int32 ShotTier = ChamberTiers[CurrentChamberIndex];
    
    // Add this debug message to show the tier
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::White, 
            FString::Printf(TEXT("Firing Tier %d Bullet!"), ShotTier));
    }

	// TODO remove that and make it not hardcoded
    float DamageMultiplier = FMath::Pow(2.5f, (float)(ShotTier - 1));
    float FinalDamage = BaseBulletDamage * DamageMultiplier;

    // 4. Trace logic
    const APawn* OwningPawn = Cast<APawn>(GetOwner());
    if (IsValid(OwningPawn))
    {
       Cast<APlayerCharacter>(OwningPawn)->PlayShootCameraShake();
    }
    
    const UCameraComponent* AimCamera = OwningPawn ? OwningPawn->FindComponentByClass<UCameraComponent>() : nullptr;
    if (!AimCamera) return;

    const FVector TraceStart = AimCamera->GetComponentLocation();
    const FVector TraceEnd = TraceStart + (AimCamera->GetForwardVector() * TraceRange);

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GunFire), true);
    QueryParams.AddIgnoredActor(this);
    if (OwningPawn) QueryParams.AddIgnoredActor(OwningPawn);

    FHitResult HitResult;
    const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
	if (bDrawDebugTrace)
	{
		DrawDebugLine(GetWorld(), TraceStart, bHit ? HitResult.ImpactPoint : TraceEnd, 
					  bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 1.0f);
	}
    // 5. Apply Damage
    if (bHit && HitResult.GetActor())
    {
       if (UHealthComponent* TargetHealth = UHealthComponent::FindHealthComponent(HitResult.GetActor()))
       {
          AController* InstigatorController = OwningPawn ? OwningPawn->GetController() : nullptr;
          TargetHealth->ApplyDamage(FinalDamage, this, InstigatorController);
       }
    }

    // 6. Consume bullet
    ChamberTiers[CurrentChamberIndex] = 0;
    CurrentAmmo = FMath::Max(CurrentAmmo - 1, 0);
    
    // 7. Visuals and Cooldown
    AddCylinderScrollInput(1.f);
    TargetRecoilLocation.X += RecoilKickBack;
    TargetRecoilRotation.Pitch += RecoilKickPitch;

    bCanFire = false;
    GetWorldTimerManager().SetTimer(FireCooldownTimerHandle, this, &ABaseGun::ResetFireCooldown, FireRate, false);
}

void ABaseGun::ResetFireCooldown()
{
	bCanFire = true;
}

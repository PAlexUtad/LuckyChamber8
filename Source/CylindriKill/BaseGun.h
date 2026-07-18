// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseGun.generated.h"

UCLASS()
class CYLINDRIKILL_API ABaseGun : public AActor
{
	GENERATED_BODY()
	
public: 
	// Sets default values for this actor's properties
	ABaseGun();

	// -------------------------------------------------------------
	// Firing (called by the player, forwarded down from input)
	// -------------------------------------------------------------

	/** Called on input press. Default (semi-auto) behaviour: fire a single shot. */
	UFUNCTION(BlueprintCallable, Category = "Gun|Fire")
	virtual void StartFire();

	/** Called on input release. No-op by default; override for automatic weapons that loop while held. */
	UFUNCTION(BlueprintCallable, Category = "Gun|Fire")
	virtual void StopFire();
	virtual void StartParry();
	virtual void StopParry();

protected:
	/** Parry configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry")
	float ParryDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry")
	float ParryCooldown = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry")
	float ParrySphereRadius = 50.f;

	bool bCanParry = true;
	bool bIsParrying = false;
	FTimerHandle ParryTimerHandle;
	FTimerHandle ParryCooldownTimerHandle;

	/** Barrel open animation rotation/offset target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry")
	FRotator BarrelOpenRotationOffset = FRotator(0.f, 0.f, -45.f);

	FRotator CurrentBarrelRotation = FRotator::ZeroRotator;
	FRotator TargetBarrelRotation = FRotator::ZeroRotator;
	/** How far (and which direction, relative to CylinderMesh's parent) the cylinder slides out during parry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry")
	FVector CylinderSlideOffset = FVector(0.f, 15.f, 0.f); // +Y = left, based on this mesh's local gizmo

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry")
	float CylinderSlideInterpSpeed = 12.f;

	FVector CurrentCylinderSlideLocation = FVector::ZeroVector;
	FVector TargetCylinderSlideLocation = FVector::ZeroVector;
	FVector CylinderBaseLoc = FVector::ZeroVector;
	
	void PerformParryTrace();
	void ResetParryCooldown();
	void EndParry();

	// Add Ammunition 
	void AddAmmo();
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Performs the actual hitscan trace and applies cooldown/ammo. Override to customize behaviour or VFX. */
	UFUNCTION(BlueprintCallable, Category = "Gun|Fire")
	virtual void Fire();

	void ResetFireCooldown();

	// -------------------------------------------------------------
	// Components
	// -------------------------------------------------------------

	/** Root scene component - keeps the actor's transform independent of any single mesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> GunRoot;

	/** Main body mesh. Subclasses attach any extra parts (barrels, cylinders, etc.) to this. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	/** Cylinder/revolver mesh — created and attached in subclass constructor (e.g. ACylinderGun). Slides during parry. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> CylinderMesh;

	// -------------------------------------------------------------
	// Fire tuning
	// -------------------------------------------------------------

	/** Seconds between shots. Also functions as the "rate of fire" cooldown gate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Fire")
	float FireRate = 0.35f;

	/** Max range of the hitscan trace, in uu. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Fire")
	float TraceRange = 10000.f;

	/** How many rounds this gun holds before it can no longer fire. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Fire")
	int32 MaxAmmo = 8;

	/** Rounds currently loaded. Reset to MaxAmmo in BeginPlay. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun|Fire")
	int32 CurrentAmmo = 8;

	/** If true, draw a debug line/sphere for each shot so hits are visible instantly in the viewport. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Debug")
	bool bDrawDebugTrace = true;

	bool bCanFire = true;
	FTimerHandle FireCooldownTimerHandle;
	/** Target and current recoil offsets for smooth spring recovery */
	FVector TargetRecoilLocation = FVector::ZeroVector;
	FVector CurrentRecoilLocation = FVector::ZeroVector;
	FRotator TargetRecoilRotation = FRotator::ZeroRotator;
	FRotator CurrentRecoilRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Recoil")
	float RecoilKickBack = -15.f; // Distance back in uu

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Recoil")
	float RecoilKickPitch = -8.f; // Degrees pitch upward

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Recoil")
	float RecoilInterpSpeed = 15.f;
public: 
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
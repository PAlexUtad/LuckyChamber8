// 
// PlayerCharacter.h
// 
// Lightweight, template-free first-person character built for a hyper-fast arcade shooter.
// - No skeletal mesh / arms - camera-only first-person view.
// - Ultrakill-style movement: instant acceleration, zero slide on stop.
// - Directional dash (ground + air) with cooldown.
// - Procedural camera bob driven by real lateral velocity.
// - Wall slide: slowed fall + camera lean when airborne next to a wall, with a boosted wall jump.
// 
// ----------------------------------------  x  ---------------------------------------- 
// 
// © 2026 CylindriKill. All rights reserved.
// 

#pragma once

#include "CoreMinimal.h"
#include "CylindriKill/BaseCharacter.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

class UCameraComponent;
class UCameraShakeBase;
class UChildActorComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class CYLINDRIKILL_API APlayerCharacter : public ABaseCharacter
{
    GENERATED_BODY()

	// ------------------------------------------------------------------
	// Internal Variables
	// ------------------------------------------------------------------
	bool     bIsWallSliding = false;
	float    BobCycleTime = 0.f;
	FVector  CameraBaseRelativeLocation = FVector::ZeroVector;
	FVector  CurrentBobOffset = FVector::ZeroVector;
	float    CurrentCameraRollOffset = 0.f;
	FRotator CurrentGunSlideRotationOffset = FRotator::ZeroRotator;
	float    CurrentSlideCameraOffset = 0.f;
	FVector  CurrentWallNormal = FVector::ZeroVector;
	float    WallJumpCooldownRemaining = 0.f;
	FVector  WeaponRelativeLocation = FVector::ZeroVector;
	FRotator WeaponRelativeRotation = FRotator::ZeroRotator;
	
	// ------------------------------------------------------------------
	// Blueprint Variables
	// ------------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UChildActorComponent> WeaponComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", Meta = (AllowPrivateAccess = "true"))
	float BobAmplitude;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", Meta = (AllowPrivateAccess = "true"))
	float BobFrequency;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", Meta = (AllowPrivateAccess = "true"))
	float BobInterpSpeed;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", Meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCameraShakeBase> ShootCameraShakeClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DashAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ParryAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ShootAction;

	// ------------------------------------------------------------------
    // Slide (post-dash ground slide)
    // ------------------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", Meta = (AllowPrivateAccess = "true"))
    float SlideCameraDropAmount;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", Meta = (AllowPrivateAccess = "true"))
    float SlideCameraInterpSpeed;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", Meta = (AllowPrivateAccess = "true"))
    float SlideWeaponInterpSpeed;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", Meta = (AllowPrivateAccess = "true"))
    float SlideWeaponPitchDegrees;
    
    // ------------------------------------------------------------------
    // Wall Slide
    // ------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", Meta = (AllowPrivateAccess = "true"))
	bool bDrawDebugWallTrace;
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", Meta = (AllowPrivateAccess = "true"))
    float WallJumpAwayStrength;
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", Meta = (AllowPrivateAccess = "true"))
    float WallJumpReattachCooldown;
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", Meta = (AllowPrivateAccess = "true"))
    float WallJumpUpStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", Meta = (AllowPrivateAccess = "true"))
	float WallSlideCameraRollDegrees;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", Meta = (AllowPrivateAccess = "true"))
	float WallSlideCameraRollInterpSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", Meta = (AllowPrivateAccess = "true"))
	float WallSlideGravityScale;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", Meta = (AllowPrivateAccess = "true"))
	float WallTraceDistance;
	
public:
    
	// ------------------------------------------------------------------
	// Constructor & Destructor
	// ------------------------------------------------------------------
    APlayerCharacter();
	
	// ------------------------------------------------------------------
	// Overridden Methods
	// ------------------------------------------------------------------
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void Tick(const float DeltaTime) override;
	virtual void Jump() override;
	
private:

    // ------------------------------------------------------------------
    // Internal Methods
    // ------------------------------------------------------------------
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void Parry();
    void Shoot();
	
public:
    
	// TODO: Shit functions that shouldn't be in the PlayerCharacter.
	bool DetectWall(FVector& OutWallNormal) const;
	void PlayShootCameraShake() const;
	void UpdateCameraBob(const float DeltaTime);
	void UpdateCameraRotation(const float DeltaTime) const;
	void UpdateSlideVisuals(const float DeltaTime);
	void UpdateWallSlide(const float DeltaTime);
	void WallJump();
	
	// ------------------------------------------------------------------
	// Getters & Setters
	// ------------------------------------------------------------------
    FORCEINLINE TObjectPtr<UChildActorComponent> GetWeaponComponent() const { return WeaponComponent; }
};
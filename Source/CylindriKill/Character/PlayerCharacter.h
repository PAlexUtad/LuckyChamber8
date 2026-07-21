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

public:
    
    APlayerCharacter();

    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Shake")
    TSubclassOf<UCameraShakeBase> ShootCameraShakeClass;

    void PlayShootCameraShake() const;

protected:

    // ------------------------------------------------------------------
    // Components
    // ------------------------------------------------------------------
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FirstPersonCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UChildActorComponent> GunChildComponent;

    FVector GunBaseRelativeLocation = FVector::ZeroVector;
    FRotator GunBaseRelativeRotation = FRotator::ZeroRotator;

    // ------------------------------------------------------------------
    // Enhanced Input Assets (assign in the editor on the BP subclass)
    // ------------------------------------------------------------------
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> JumpAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> DashAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> FireAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> ParryAction;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> ScrollAction;
    
    // ------------------------------------------------------------------
    // Input Handlers
    // ------------------------------------------------------------------
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartParry();
    void Scroll(const FInputActionValue& Value);
    void StartFire();
    void StopFire();
    
    // ------------------------------------------------------------------
    // Slide (post-dash ground slide)
    // ------------------------------------------------------------------
    float CurrentSlideCameraOffset = 0.f;
    FRotator CurrentGunSlideRotationOffset = FRotator::ZeroRotator;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"))
    float SlideCameraDropAmount;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"))
    float SlideCameraInterpSpeed;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"))
    float SlideWeaponInterpSpeed;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"))
    float SlideWeaponPitchDegrees;
    
    // ------------------------------------------------------------------
    // Wall Slide
    // ------------------------------------------------------------------

    /** How far out (in uu) the radial wall-check traces reach from the capsule. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallSlide")
    float WallTraceDistance = 60.f;

    /** Gravity scale applied while attached to a wall - lower falls slower. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallSlide")
    float WallSlideGravityScale = 0.3f;

    /** How far (degrees) the camera rolls/leans while wall-sliding. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallSlide")
    float WallSlideCameraRollDegrees = 15.f;

    /** How quickly the camera roll interpolates in/out of the lean. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallSlide")
    float WallSlideCameraRollInterpSpeed = 10.f;

    /** Outward (away-from-wall) launch strength for a wall jump. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallSlide")
    float WallJumpAwayStrength = 900.f;

    /** Upward launch strength for a wall jump. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallSlide")
    float WallJumpUpStrength = 700.f;

    /** Prevents instantly re-attaching to the same wall right after a wall jump. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallSlide")
    float WallJumpReattachCooldown = 0.3f;

    /** Draw the radial wall-detection traces for tuning. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallSlide|Debug")
    bool bDrawDebugWallTrace = false;
    
    bool bIsWallSliding = false;
    FVector CurrentWallNormal = FVector::ZeroVector;
    float CurrentCameraRollOffset = 0.f;
    float WallJumpCooldownRemaining = 0.f;

    void UpdateWallSlide(float DeltaTime);
    bool DetectWall(FVector& OutWallNormal) const;
    void WallJump();

    // ------------------------------------------------------------------
    // Procedural Camera Bob & Rotation
    // ------------------------------------------------------------------

    /** How fast the bob cycle advances at full speed. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Bob")
    float BobFrequency = 14.f;

    /** Max vertical/horizontal displacement of the bob, in uu. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Bob")
    float BobAmplitude = 6.f;

    /** How quickly the bob offset interpolates towards its target (higher = snappier). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Bob")
    float BobInterpSpeed = 10.f;

    FVector CameraBaseRelativeLocation = FVector::ZeroVector;
    float BobCycleTime = 0.f;
    FVector CurrentBobOffset = FVector::ZeroVector;

    void UpdateCameraBob(float DeltaTime);

    /** Manually drives camera pitch/yaw (from control rotation) + roll (from wall lean) every tick.
     *  Needed because FirstPersonCamera->bUsePawnControlRotation is now false - see UpdateWallSlide. */
    void UpdateCameraRotation(float DeltaTime);

    // JUMP
    virtual auto Jump() -> void override;

    UFUNCTION()
    void HandleDeath(AActor* DamageCauser);
    
    void UpdateSlideVisuals(float DeltaTime);
    
public:
    
    FORCEINLINE TObjectPtr<UChildActorComponent> GetWeaponComponent() const { return GunChildComponent; }
};
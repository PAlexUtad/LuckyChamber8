// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

class UCameraComponent;
class UChildActorComponent;
class UInputMappingContext;
class UInputAction;
class UCameraShakeBase;

/**
 * APlayerCharacter
 *
 * Lightweight, template-free first-person character built for a hyper-fast arcade shooter.
 * - No skeletal mesh / arms - camera-only first-person view.
 * - Ultrakill-style movement: instant acceleration, zero slide on stop.
 * - Directional dash (ground + air) with cooldown.
 * - Procedural camera bob driven by real lateral velocity.
 * - Wall slide: slowed fall + camera lean when airborne next to a wall, with a boosted wall jump.
 */
UCLASS()
class CYLINDRIKILL_API APlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();

    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Shake")
    TSubclassOf<UCameraShakeBase> ShootCameraShakeClass;

    void PlayShootCameraShake() const;

protected:
    virtual void BeginPlay() override;

    // ------------------------------------------------------------------
    // Components
    // ------------------------------------------------------------------

    /** First-person camera, attached directly to the capsule at eye height. No spring arm - zero lag. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FirstPersonCamera;

    /** Child actor component that holds and displays the equipped gun blueprint. */
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
    void StartDash();
    void Scroll(const FInputActionValue& Value);
    /** Forwards the fire-pressed input down to whatever gun is currently equipped. */
    void StartFire();

    /** Forwards the fire-released input down to whatever gun is currently equipped. */
    void StopFire();

    // ------------------------------------------------------------------
    // Dash
    // ------------------------------------------------------------------

    /** Instantaneous horizontal speed applied on dash (uu/s). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash")
    float DashImpulseStrength = 2500.f;

    /** Cooldown between dashes, in seconds. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash")
    float DashCooldownDuration = 0.75f;

    bool bCanDash = true;
    FTimerHandle DashCooldownTimerHandle;
    void ResetDashCooldown();

    /** Cached raw 2D move input (X = right/left, Y = forward/back). Used to derive dash direction. */
    FVector2D LastMoveInput = FVector2D::ZeroVector;

    // ------------------------------------------------------------------
    // Slide (post-dash ground slide)
    // ------------------------------------------------------------------

    /** How long the slide state (low friction + lowered camera + tilted gun) lasts after a dash. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
    float SlideDuration = 0.75f;

    /** Ground friction used only while sliding - low, so the dash burst actually carries the player. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
    float SlideGroundFriction = 0.1f;

    /** Braking deceleration used only while sliding - low, so speed bleeds off gradually, not instantly. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
    float SlideBrakingDeceleration = 100.f;

    /** Braking friction lowered during slide so standing-still slides actually carry. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
    float SlideBrakingFriction = 0.0f;

    /** Lower acceleration during slide so held WASD keys don't instantly yank/kill your momentum. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
    float SlideMaxAcceleration = 500.f;

    /** How far the camera drops (in uu) at the peak of the slide. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
    float SlideCameraDropAmount = 40.f;

    /** How quickly the camera interpolates towards/away from the slide drop. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
    float SlideCameraInterpSpeed = 10.f;

    /** Pitch (in degrees) the gun rotates to while sliding, relative to its resting rotation. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
    float SlideGunPitchDegrees = -65.f;

    /** How quickly the gun interpolates towards/away from its slide pitch. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
    float SlideGunInterpSpeed = 10.f;

    bool bIsSliding = false;
    FTimerHandle SlideTimerHandle;
    void EndSlide();

    /** Movement values captured once in BeginPlay so EndSlide() can restore the "normal" feel exactly. */
    float DefaultGroundFriction = 0.f;
    float DefaultBrakingDecelerationWalking = 0.f;
    float DefaultBrakingFriction = 0.f;
    float DefaultMaxAcceleration = 0.f;
    float DefaultGravityScale = 1.6f;

    float CurrentSlideCameraOffset = 0.f;
    FRotator CurrentGunSlideRotationOffset = FRotator::ZeroRotator;
    void UpdateSlideVisuals(float DeltaTime);

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


    // Health Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UHealthComponent> HealthComponent;

    UFUNCTION()
    void HandleDeath(AActor* DamageCauser);
};
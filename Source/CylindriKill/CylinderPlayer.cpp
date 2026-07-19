// Fill out your copyright notice in the Description page of Project Settings.

#include "CylinderPlayer.h"

#include "BaseGun.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ChildActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "TimerManager.h"
#include "Camera/CameraShakeBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "DrawDebugHelpers.h"

ACylinderPlayer::ACylinderPlayer()
{
    PrimaryActorTick.bCanEverTick = true;

    // -------------------------------------------------------------
    // No skeletal mesh / arms - keep the pawn lightweight. The mesh
    // component still exists (ACharacter requires one internally) but
    // we never assign an asset to it and hide it entirely.
    // -------------------------------------------------------------
    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
       MeshComp->SetHiddenInGame(true);
       MeshComp->SetCastShadow(false);
       MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    GetCapsuleComponent()->InitCapsuleSize(34.f, 90.f);

    // -------------------------------------------------------------
    // Camera - attached directly to the capsule, no spring arm.
    // bUsePawnControlRotation is OFF: we now drive pitch/yaw/roll manually
    // every tick in UpdateCameraRotation(), so the wall-slide lean (roll)
    // isn't silently overwritten by the engine's own control-rotation sync.
    // -------------------------------------------------------------
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(RootComponent);
    FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 74.f)); // approx eye height
    FirstPersonCamera->bUsePawnControlRotation = false;

    // -------------------------------------------------------------
    // Gun Child Actor Component - attached directly to the camera
    // -------------------------------------------------------------
    GunChildComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("GunChildComponent"));
    GunChildComponent->SetupAttachment(FirstPersonCamera);

    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    // -------------------------------------------------------------
    // Ultrakill-style movement tuning:
    // High acceleration + high friction + extreme braking deceleration
    // = the character reaches top speed almost instantly and stops
    // dead the moment input is released. No skating, no slide.
    // -------------------------------------------------------------
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->bOrientRotationToMovement = false;

    MoveComp->MaxWalkSpeed = 1200.f;
    MoveComp->MaxAcceleration = 8192.f;              // near-instant ramp to top speed
    MoveComp->GroundFriction = 16.f;                 // crisp direction changes
    MoveComp->BrakingDecelerationWalking = 8192.f;   // stop instantly, no sliding
    MoveComp->bUseSeparateBrakingFriction = true;
    MoveComp->BrakingFriction = 16.f;
    MoveComp->BrakingFrictionFactor = 1.f;

    MoveComp->AirControl = 1.f;                      // full control while airborne
    MoveComp->FallingLateralFriction = 0.f;
    MoveComp->BrakingDecelerationFalling = 0.f;       // preserve air momentum (dash/jump flow)
    MoveComp->JumpZVelocity = 600.f;
    MoveComp->GravityScale = 1.6f;                    // snappier arcs, less floaty

    MoveComp->NavAgentProps.bCanCrouch = false;
}

void ACylinderPlayer::BeginPlay()
{
    Super::BeginPlay();

    // Register our Enhanced Input mapping context with the local player subsystem.
    if (const APlayerController* PC = Cast<APlayerController>(GetController()))
    {
       if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
             ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
       {
          if (DefaultMappingContext)
          {
             Subsystem->AddMappingContext(DefaultMappingContext, 0);
          }
       }
    }

    // Cache the camera's resting position so bobbing offsets are always relative to it.
    if (FirstPersonCamera)
    {
       CameraBaseRelativeLocation = FirstPersonCamera->GetRelativeLocation();
    }

    // Explicitly set the gun actor's owner to this player character
    if (IsValid(GunChildComponent))
    {
       GunBaseRelativeLocation = GunChildComponent->GetRelativeLocation();
       GunBaseRelativeRotation = GunChildComponent->GetRelativeRotation();
       if (AActor* ChildActor = GunChildComponent->GetChildActor())
       {
          ChildActor->SetOwner(this);
       }
    }

    // Cache the movement component's "normal" ground/air feel so EndSlide()/wall-slide-exit
    // can restore it exactly, even if these values are later tweaked in a Blueprint's class defaults.
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
       DefaultGroundFriction = MoveComp->GroundFriction;
       DefaultBrakingDecelerationWalking = MoveComp->BrakingDecelerationWalking;
       DefaultBrakingFriction = MoveComp->BrakingFriction;
       DefaultMaxAcceleration = MoveComp->MaxAcceleration;
       DefaultGravityScale = MoveComp->GravityScale;
    }
}

void ACylinderPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateSlideVisuals(DeltaTime);
    UpdateCameraBob(DeltaTime);
    UpdateWallSlide(DeltaTime);
    UpdateCameraRotation(DeltaTime); // must run last - applies pitch/yaw/roll after everything else has decided its inputs
}

void ACylinderPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
       if (MoveAction)
       {
          EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACylinderPlayer::Move);
          EIC->BindAction(MoveAction, ETriggerEvent::Completed, this, &ACylinderPlayer::Move);
       }

       if (LookAction)
       {
          EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACylinderPlayer::Look);
       }

       if (JumpAction)
       {
          EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
          EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
       }

       if (DashAction)
       {
          EIC->BindAction(DashAction, ETriggerEvent::Started, this, &ACylinderPlayer::StartDash);
       }

       if (FireAction)
       {
          EIC->BindAction(FireAction, ETriggerEvent::Started, this, &ACylinderPlayer::StartFire);
          EIC->BindAction(FireAction, ETriggerEvent::Completed, this, &ACylinderPlayer::StopFire);
       }
       if (ParryAction)
       {
          EIC->BindAction(ParryAction, ETriggerEvent::Started, this, &ACylinderPlayer::StartParry);
       }
       if (ScrollAction)
       {
          EIC->BindAction(ScrollAction, ETriggerEvent::Triggered, this, &ACylinderPlayer::Scroll);
       }
    }
}

void ACylinderPlayer::Move(const FInputActionValue& Value)
{
    const FVector2D InputVector = Value.Get<FVector2D>();
    LastMoveInput = InputVector; // cached for dash direction; becomes (0,0) on the Completed event

    if (Controller)
    {
       // Movement is relative to camera yaw only (pitch/roll ignored) so looking up/down
       // never affects ground movement speed or direction.
       const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
       const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
       const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

       AddMovementInput(ForwardDirection, InputVector.Y);
       AddMovementInput(RightDirection, InputVector.X);
    }
}

void ACylinderPlayer::Look(const FInputActionValue& Value)
{
    const FVector2D LookAxis = Value.Get<FVector2D>();

    if (Controller)
    {
       AddControllerYawInput(LookAxis.X);
       // If mouse-up rotates the camera downward, add a "Negate" modifier to the Y axis
       // on the IA_Look asset (standard Enhanced Input convention) rather than flipping here.
       AddControllerPitchInput(LookAxis.Y);
    }
}
void ACylinderPlayer::Scroll(const FInputActionValue& Value)
{
   if (GunChildComponent)
   {
      if (ABaseGun* Gun = Cast<ABaseGun>(GunChildComponent->GetChildActor()))
      {
         Gun->AddCylinderScrollInput(Value.Get<float>());
      }
   }
}
void ACylinderPlayer::StartParry()
{
   if (GunChildComponent)
   {
      if (ABaseGun* Gun = Cast<ABaseGun>(GunChildComponent->GetChildActor()))
      {
         Gun->StartParry();
      }
   }
}

void ACylinderPlayer::StartDash()
{
    if (!bCanDash)
    {
       return;
    }

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp)
    {
       return;
    }

    // Resolve direction: WASD input direction if pressed, otherwise look direction (camera forward).
    const float ControlYaw = Controller ? Controller->GetControlRotation().Yaw : GetActorRotation().Yaw;
    const FRotator YawRotation(0.f, ControlYaw, 0.f);
    const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    FVector SlideDirection;
    if (!LastMoveInput.IsNearlyZero())
    {
       // Direction of movement based on keys held
       SlideDirection = (Forward * LastMoveInput.Y + Right * LastMoveInput.X).GetSafeNormal();
    }
    else
    {
       // Direction of the camera/view if standing still
       SlideDirection = Forward;
    }
    SlideDirection.Z = 0.f;
    SlideDirection.Normalize();

    // Apply an even stronger burst if you want it faster/further out of the gate
    const FVector LaunchVelocity(
       SlideDirection.X * DashImpulseStrength,
       SlideDirection.Y * DashImpulseStrength,
       MoveComp->Velocity.Z + 200.f); // slight upward pop for flow

    LaunchCharacter(LaunchVelocity, true, false);

    // Loosen all movement gates so the physics burst isn't immediately murdered
    if (MoveComp->IsMovingOnGround())
    {
       MoveComp->GroundFriction = SlideGroundFriction;
       MoveComp->BrakingDecelerationWalking = SlideBrakingDeceleration;
       MoveComp->BrakingFriction = SlideBrakingFriction;
       MoveComp->MaxAcceleration = SlideMaxAcceleration;
    }

    bIsSliding = true;
    GetWorldTimerManager().SetTimer(
       SlideTimerHandle,
       this,
       &ACylinderPlayer::EndSlide,
       SlideDuration,
       false);

    bCanDash = false;
    GetWorldTimerManager().SetTimer(
       DashCooldownTimerHandle,
       this,
       &ACylinderPlayer::ResetDashCooldown,
       DashCooldownDuration,
       false);
}

void ACylinderPlayer::ResetDashCooldown()
{
    bCanDash = true;
}

void ACylinderPlayer::EndSlide()
{
   bIsSliding = false;

   if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
   {
      MoveComp->GroundFriction = DefaultGroundFriction;
      MoveComp->BrakingDecelerationWalking = DefaultBrakingDecelerationWalking;
      MoveComp->BrakingFriction = DefaultBrakingFriction;
      MoveComp->MaxAcceleration = DefaultMaxAcceleration;
   }
}

void ACylinderPlayer::StartFire()
{
   if (!GunChildComponent)
   {
      GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("StartFire: GunChildComponent is null!"));
      return;
   }

   AActor* ChildActor = GunChildComponent->GetChildActor();
   if (!ChildActor)
   {
      GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("StartFire: ChildActor is null! Is Child Actor Class assigned?"));
      return;
   }

   if (ABaseGun* Gun = Cast<ABaseGun>(ChildActor))
   {
      GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("StartFire: Calling Gun->StartFire()"));

      Gun->StartFire();
   }
   else
   {
      GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("StartFire: Failed to cast ChildActor to ABaseGun!"));
   }
}

void ACylinderPlayer::StopFire()
{
    if (!GunChildComponent)
    {
       return;
    }

    if (ABaseGun* Gun = Cast<ABaseGun>(GunChildComponent->GetChildActor()))
    {
       Gun->StopFire();
    }
}

void ACylinderPlayer::Jump()
{
   // If we are currently sliding, cancel the slide immediately on jump press
   if (bIsSliding)
   {
      EndSlide();
   }

   // If wall-sliding, launch off the wall instead of a normal jump
   if (bIsWallSliding)
   {
      WallJump();
      return;
   }

   // Call the parent character's standard jump logic
   Super::Jump();
}

void ACylinderPlayer::PlayShootCameraShake() const
{
   if (ShootCameraShakeClass && Controller)
   {
      if (APlayerController* PC = Cast<APlayerController>(Controller))
      {
         PC->ClientStartCameraShake(ShootCameraShakeClass, 1.0f);
      }
   }
}

void ACylinderPlayer::UpdateCameraBob(float DeltaTime)
{
    if (!FirstPersonCamera)
    {
       return;
    }

    // Only lateral (horizontal) speed drives the bob - vertical velocity from
    // jumping/falling/dashing shouldn't feed into it.
    FVector Velocity = GetVelocity();
    Velocity.Z = 0.f;
    const float Speed = Velocity.Size();

    const float MaxSpeed = GetCharacterMovement()->MaxWalkSpeed;
    const float SpeedRatio = (MaxSpeed > 0.f) ? FMath::Clamp(Speed / MaxSpeed, 0.f, 1.f) : 0.f;

    FVector TargetOffset = FVector::ZeroVector;

    // While sliding, the smooth camera drop (handled in UpdateSlideVisuals) takes over -
    // footstep bob would fight it and look jittery on top of a slide.
    const bool bShouldBob = SpeedRatio > KINDA_SMALL_NUMBER && GetCharacterMovement()->IsMovingOnGround() && !bIsSliding;
    if (bShouldBob)
    {
       // Advance the cycle at a rate proportional to current speed, so faster
       // movement produces both a higher amplitude AND a faster bob rhythm.
       BobCycleTime += DeltaTime * BobFrequency * SpeedRatio;

       // Vertical bob completes two cycles per horizontal cycle (classic footstep
       // figure-8 pattern: down-up-down-up per left-right-left-right stride).
       const float VerticalBob = FMath::Sin(BobCycleTime * 2.f) * BobAmplitude * SpeedRatio;
       const float HorizontalBob = FMath::Cos(BobCycleTime) * (BobAmplitude * 0.5f) * SpeedRatio;

       TargetOffset = FVector(0.f, HorizontalBob, VerticalBob);
    }
    else
    {
       // Reset the cycle so the next bob always starts from a clean phase.
       BobCycleTime = 0.f;
    }

    // Interpolate rather than snap, so speeding up/slowing down or landing never pops the camera.
    CurrentBobOffset = FMath::VInterpTo(CurrentBobOffset, TargetOffset, DeltaTime, BobInterpSpeed);

    // Combine footstep bob with the slide's vertical drop (computed in UpdateSlideVisuals).
    FirstPersonCamera->SetRelativeLocation(
       CameraBaseRelativeLocation + CurrentBobOffset + FVector(0.f, 0.f, CurrentSlideCameraOffset));

   if (IsValid(GunChildComponent))
   {
      // Push the gun opposite to the camera bob offset, scaled down slightly so it feels heavy
      GunChildComponent->SetRelativeLocation(GunBaseRelativeLocation - (CurrentBobOffset * 0.75f));
   }
}

void ACylinderPlayer::UpdateSlideVisuals(float DeltaTime)
{
    // -------------------------------------------------------------
    // Camera: smoothly lower while sliding (like "The Finals"), smoothly
    // rise back to normal once the slide ends. The actual displacement is
    // applied in UpdateCameraBob so it composes cleanly with footstep bob.
    // -------------------------------------------------------------
    const float TargetCameraSlideOffset = bIsSliding ? -SlideCameraDropAmount : 0.f;
    CurrentSlideCameraOffset = FMath::FInterpTo(
       CurrentSlideCameraOffset, TargetCameraSlideOffset, DeltaTime, SlideCameraInterpSpeed);

    // -------------------------------------------------------------
    // Gun: pitch up towards the sky while sliding, settle back down to its
    // normal aiming rotation once the slide ends.
    // -------------------------------------------------------------
    const FRotator TargetGunSlideRotation = bIsSliding ? FRotator(SlideGunPitchDegrees, 0.f, 0.f) : FRotator::ZeroRotator;
    CurrentGunSlideRotationOffset = FMath::RInterpTo(
       CurrentGunSlideRotationOffset, TargetGunSlideRotation, DeltaTime, SlideGunInterpSpeed);

    if (IsValid(GunChildComponent))
    {
       GunChildComponent->SetRelativeRotation(GunBaseRelativeRotation + CurrentGunSlideRotationOffset);
    }
}

bool ACylinderPlayer::DetectWall(FVector& OutWallNormal) const
{
    const FVector Start = GetActorLocation();

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WallSlideTrace), false);
    QueryParams.AddIgnoredActor(this);

    static constexpr int32 NumDirections = 8;
    for (int32 i = 0; i < NumDirections; ++i)
    {
       const float Angle = (360.f / NumDirections) * i;
       const FVector Dir = FRotator(0.f, Angle, 0.f).RotateVector(FVector::ForwardVector);
       const FVector End = Start + Dir * WallTraceDistance;

       FHitResult Hit;
       const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams);

       if (bDrawDebugWallTrace)
       {
          DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 0.f, 0, 1.f);
       }

       if (bHit)
       {
          OutWallNormal = Hit.Normal;
          return true;
       }
    }
    return false;
}

void ACylinderPlayer::UpdateWallSlide(float DeltaTime)
{
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp)
    {
       return;
    }

    if (WallJumpCooldownRemaining > 0.f)
    {
       WallJumpCooldownRemaining -= DeltaTime;
    }

    const bool bFalling = MoveComp->IsFalling();
    const bool bMovingDown = GetVelocity().Z < 0.f;

    FVector WallNormal;
    const bool bWallDetected = bFalling && bMovingDown && WallJumpCooldownRemaining <= 0.f && DetectWall(WallNormal);

    bIsWallSliding = bWallDetected;

    if (bIsWallSliding)
    {
       CurrentWallNormal = WallNormal;
       MoveComp->GravityScale = WallSlideGravityScale;
    }
    else
    {
       MoveComp->GravityScale = DefaultGravityScale;
    }

    // Lean the camera toward the wall - use camera yaw (not actor yaw), since the capsule
    // never rotates with the mouse (bUseControllerRotationYaw is false on this pawn).
    float TargetRoll = 0.f;
    if (bIsWallSliding && Controller)
    {
       const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
       const FVector CameraRight = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
       const float Side = FVector::DotProduct(CurrentWallNormal, CameraRight);
       TargetRoll = (Side > 0.f) ? WallSlideCameraRollDegrees : -WallSlideCameraRollDegrees;
    }

    CurrentCameraRollOffset = FMath::FInterpTo(CurrentCameraRollOffset, TargetRoll, DeltaTime, WallSlideCameraRollInterpSpeed);
}

void ACylinderPlayer::WallJump()
{
    FVector LaunchVelocity = CurrentWallNormal * WallJumpAwayStrength;
    LaunchVelocity.Z = WallJumpUpStrength;

    LaunchCharacter(LaunchVelocity, true, true);

    bIsWallSliding = false;
    WallJumpCooldownRemaining = WallJumpReattachCooldown;

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
       MoveComp->GravityScale = DefaultGravityScale;
    }
}

void ACylinderPlayer::UpdateCameraRotation(float DeltaTime)
{
    if (!FirstPersonCamera || !Controller)
    {
       return;
    }

    FRotator DesiredRotation = Controller->GetControlRotation();
    DesiredRotation.Roll = CurrentCameraRollOffset;
    FirstPersonCamera->SetWorldRotation(DesiredRotation);
}
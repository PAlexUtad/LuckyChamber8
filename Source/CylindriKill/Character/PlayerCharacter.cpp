// 
// PlayerCharacter.cpp
// 
// Implementation of the 'PlayerCharacter' class.
// 
// ----------------------------------------  x  ---------------------------------------- 
// 
// © 2026 CylindriKill. All rights reserved.
// 

#include "PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CylindriKill/Ability/DashAbility.h"
#include "CylindriKill/BaseWeapon.h"
#include "CylindriKill/Component/HealthComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "TimerManager.h"
#include "Util/LoggerUtil.h"

// ------------------------------------------------------------------
// Constructor & Destructor
// ------------------------------------------------------------------
APlayerCharacter::APlayerCharacter()
{
	BobAmplitude   = 6.0f;
	BobFrequency   = 14.0f;
	BobInterpSpeed = 10.0f;
	
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(RootComponent);
    CameraComponent->SetRelativeLocation(FVector(0.f, 0.f, 74.f));
    CameraComponent->bUsePawnControlRotation = false;
	
    GetCapsuleComponent()->InitCapsuleSize(34.f, 90.f);

	HealthComponent->MaxHealth = 100.f;
	
	const TObjectPtr<UCharacterMovementComponent> MovementComponent = GetCharacterMovement();
	
	MovementComponent->AirControl = 1.0f;
	MovementComponent->BrakingDecelerationFalling = 0.0f;
	MovementComponent->BrakingDecelerationWalking = 8192.f;
	MovementComponent->BrakingFriction = 16.0f;
	MovementComponent->BrakingFrictionFactor = 1.0f;
	MovementComponent->bOrientRotationToMovement = false;
	MovementComponent->bUseSeparateBrakingFriction = true;
	MovementComponent->FallingLateralFriction = 0.0f;
	MovementComponent->GravityScale = 1.6f;
	MovementComponent->GroundFriction = 16.0f;
	MovementComponent->JumpZVelocity = 600.0f;
	MovementComponent->MaxWalkSpeed = 1200.0f;
	MovementComponent->MaxAcceleration = 8192.0f;
	MovementComponent->NavAgentProps.bCanCrouch = false;
	
	PrimaryActorTick.bCanEverTick = true;
	
	if (const TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent = GetMesh())
	{
		SkeletalMeshComponent->SetHiddenInGame(true);
		SkeletalMeshComponent->SetCastShadow(false);
		SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	SlideCameraDropAmount   = 40.f;
	SlideCameraInterpSpeed  = 10.f;
	SlideWeaponInterpSpeed  = 10.f;
	SlideWeaponPitchDegrees = -65.f;
	
    WeaponComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("WeaponComponent"));
    WeaponComponent->SetupAttachment(CameraComponent);
	
	bDrawDebugWallTrace = false;
	WallJumpAwayStrength = 900.f;
	WallJumpReattachCooldown = 0.3f;
	WallJumpUpStrength = 700.f;
	WallSlideCameraRollDegrees = 15.f;
	WallSlideCameraRollInterpSpeed = 10.f;
	WallSlideGravityScale = 0.3f;
	WallTraceDistance = 60.f;
}

// ------------------------------------------------------------------
// Overridden Methods
// ------------------------------------------------------------------
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (const TObjectPtr<APlayerController> PC = Cast<APlayerController>(GetController()))
	{
		if (const TObjectPtr<UEnhancedInputLocalPlayerSubsystem> Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (MappingContext)
			{
				Subsystem->AddMappingContext(MappingContext, 0);
			}
		}
	}
	
	if (CameraComponent)
	{
		CameraBaseRelativeLocation = CameraComponent->GetRelativeLocation();
	}

	if (IsValid(WeaponComponent))
	{
		WeaponRelativeLocation = WeaponComponent->GetRelativeLocation();
		WeaponRelativeRotation = WeaponComponent->GetRelativeRotation();

		if (const TObjectPtr<AActor> ChildActor = WeaponComponent->GetChildActor())
			ChildActor->SetOwner(this);
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

	const TObjectPtr<UEnhancedInputComponent> EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	
	if (!EIC)
	{
		FLogger::ErrorLog("Could not initialize EnhancedInputComponent!");
		return;
	}
	
	const TSubclassOf<UBaseAbility> DashAbility = TSubclassOf<UBaseAbility>(UDashAbility::StaticClass());
	
	EIC->BindAction(DashAction,   ETriggerEvent::Started,   this, &APlayerCharacter::ActivateAbility, DashAbility);
	EIC->BindAction(JumpAction,   ETriggerEvent::Started,   this, &ACharacter::Jump);
	EIC->BindAction(JumpAction,   ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	EIC->BindAction(LookAction,   ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
	EIC->BindAction(MoveAction,   ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
	EIC->BindAction(MoveAction,   ETriggerEvent::Completed, this, &APlayerCharacter::Move);
	EIC->BindAction(ParryAction,  ETriggerEvent::Started,   this, &APlayerCharacter::Parry);
	EIC->BindAction(ShootAction,  ETriggerEvent::Started,   this, &APlayerCharacter::Shoot);
}

void APlayerCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSlideVisuals(DeltaTime);
	UpdateCameraBob(DeltaTime);
	UpdateWallSlide(DeltaTime);
	UpdateCameraRotation(DeltaTime);
}

void APlayerCharacter::Jump()
{
	// TODO: If we are currently sliding, cancel the slide immediately on jump press.
	if (bIsWallSliding)
	{
		WallJump();
		return;
	}

	Super::Jump();
}

// ------------------------------------------------------------------
// Internal Methods
// ------------------------------------------------------------------
void APlayerCharacter::Move(const FInputActionValue& Value)
{
    const FVector2D InputVector = Value.Get<FVector2D>();

    if (Controller)
    {
       const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
       const FVector  ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
       const FVector  RightDirection   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

       AddMovementInput(ForwardDirection, InputVector.Y);
       AddMovementInput(RightDirection, InputVector.X);
    }
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
    const FVector2D LookAxis = Value.Get<FVector2D>();

    if (Controller)
    {
       AddControllerYawInput(LookAxis.X);
       AddControllerPitchInput(LookAxis.Y);
    }
}

void APlayerCharacter::Parry()
{
	if (WeaponComponent)
	{
		if (const TObjectPtr<ABaseWeapon> Gun = Cast<ABaseWeapon>(WeaponComponent->GetChildActor()))
		{
			Gun->StartParry();
		}
	}
}

void APlayerCharacter::Shoot()
{
	const TObjectPtr<AActor>   Actor  = WeaponComponent->GetChildActor();
	const TObjectPtr<ABaseWeapon> Weapon = Cast<ABaseWeapon>(Actor);

	Weapon->StartFire();
}

bool APlayerCharacter::DetectWall(FVector& OutWallNormal) const
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

void APlayerCharacter::PlayShootCameraShake() const
{
   if (ShootCameraShakeClass && Controller)
   {
      if (const TObjectPtr<APlayerController> PC = Cast<APlayerController>(Controller))
      {
         PC->ClientStartCameraShake(ShootCameraShakeClass, 1.0f);
      }
   }
}

void APlayerCharacter::UpdateCameraBob(const float DeltaTime)
{
    if (!CameraComponent)
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
    TObjectPtr<UDashAbility> DashAbility = Cast<UDashAbility>(FindAbility(UDashAbility::StaticClass()));
    const bool bIsSliding = DashAbility ? DashAbility->IsOnCooldown() : false;
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
    CameraComponent->SetRelativeLocation(
       CameraBaseRelativeLocation + CurrentBobOffset + FVector(0.f, 0.f, CurrentSlideCameraOffset));

   if (IsValid(WeaponComponent))
   {
      // Push the gun opposite to the camera bob offset, scaled down slightly so it feels heavy
      WeaponComponent->SetRelativeLocation(WeaponRelativeLocation - (CurrentBobOffset * 0.75f));
   }
}

void APlayerCharacter::UpdateCameraRotation(const float DeltaTime) const
{
    if (!CameraComponent || !Controller)
    {
       return;
    }

    FRotator DesiredRotation = Controller->GetControlRotation();
    DesiredRotation.Roll = CurrentCameraRollOffset;
    CameraComponent->SetWorldRotation(DesiredRotation);
}

void APlayerCharacter::UpdateSlideVisuals(const float DeltaTime)
{
	// -------------------------------------------------------------
	// Camera: smoothly lower while sliding (like "The Finals"), smoothly
	// rise back to normal once the slide ends. The actual displacement is
	// applied in UpdateCameraBob so it composes cleanly with footstep bob.
	// -------------------------------------------------------------
	const TObjectPtr<UDashAbility> DashAbility = Cast<UDashAbility>(FindAbility(UDashAbility::StaticClass()));
	const bool bIsSliding = DashAbility ? DashAbility->IsOnCooldown() : false;
	const float TargetCameraSlideOffset = bIsSliding ? -SlideCameraDropAmount : 0.f;
	CurrentSlideCameraOffset = FMath::FInterpTo(
	  CurrentSlideCameraOffset, TargetCameraSlideOffset, DeltaTime, SlideCameraInterpSpeed);

	// -------------------------------------------------------------
	// Gun: pitch up towards the sky while sliding, settle back down to its
	// normal aiming rotation once the slide ends.
	// -------------------------------------------------------------
	const FRotator TargetGunSlideRotation = bIsSliding ? FRotator(SlideWeaponPitchDegrees, 0.f, 0.f) : FRotator::ZeroRotator;
	CurrentGunSlideRotationOffset = FMath::RInterpTo(
	  CurrentGunSlideRotationOffset, TargetGunSlideRotation, DeltaTime, SlideWeaponInterpSpeed);

	if (IsValid(WeaponComponent))
	{
	  WeaponComponent->SetRelativeRotation(WeaponRelativeRotation + CurrentGunSlideRotationOffset);
	}
}

void APlayerCharacter::UpdateWallSlide(const float DeltaTime)
{
	const TObjectPtr<UCharacterMovementComponent> MoveComp = GetCharacterMovement();
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
		MoveComp->GravityScale = 1.6f;
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

void APlayerCharacter::WallJump()
{
	FVector LaunchVelocity = CurrentWallNormal * WallJumpAwayStrength;
	LaunchVelocity.Z = WallJumpUpStrength;

	LaunchCharacter(LaunchVelocity, true, true);

	bIsWallSliding = false;
	WallJumpCooldownRemaining = WallJumpReattachCooldown;

	if (const TObjectPtr<UCharacterMovementComponent> MoveComp = GetCharacterMovement())
	{
		MoveComp->GravityScale = 1.6f;
	}
}

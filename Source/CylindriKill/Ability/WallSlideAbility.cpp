// 
// WallSlideAbility.h
// 
// Implementation of the 'WallSlideAbility' class.
// 
// ----------------------------------------  x  ---------------------------------------- 
// 
// © 2026 CylindriKill. All rights reserved.
// 

#include "WallSlideAbility.h"

#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "CylindriKill/BaseCharacter.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

// ------------------------------------------------------------------
// Constructor & Destructor
// ------------------------------------------------------------------
UWallSlideAbility::UWallSlideAbility()
{
	bIsWallSliding				   = false;
	CurrentWallNormal              = FVector::ZeroVector;
	WallJumpCooldownRemaining      = 0.0f;
	
	bDrawDebugWallTrace            = false;
	WallJumpAwayStrength           = 900.f;
	WallJumpReattachCooldown       = 0.3f;
	WallJumpUpStrength             = 700.f;
	WallSlideCameraRollDegrees     = 15.f;
	WallSlideCameraRollInterpSpeed = 10.f;
	WallSlideGravityScale          = 0.3f;
	WallTraceDistance              = 60.f;
}

// ------------------------------------------------------------------
// Exposed Methods
// ------------------------------------------------------------------
bool UWallSlideAbility::Activate()
{
	if (!Super::Activate())
		return false;
	
	const TObjectPtr<ABaseCharacter> Owner = GetTypedOuter<ABaseCharacter>();
	FVector LaunchVelocity = CurrentWallNormal * WallJumpAwayStrength;
	LaunchVelocity.Z = WallJumpUpStrength;

	Owner->LaunchCharacter(LaunchVelocity, true, true);

	bIsWallSliding = false;
	WallJumpCooldownRemaining = WallJumpReattachCooldown;

	if (const TObjectPtr<UCharacterMovementComponent> MoveComp = Owner->GetCharacterMovement())
		MoveComp->GravityScale = 1.6f;
	
	return true;
}

bool UWallSlideAbility::IsInProgress() const
{
	return bIsWallSliding;
}

// ------------------------------------------------------------------
// Internal Methods
// ------------------------------------------------------------------
bool UWallSlideAbility::DetectWall(FVector& OutWallNormal) const
{
	const TObjectPtr<ABaseCharacter> Owner = GetTypedOuter<ABaseCharacter>(); 
	const FVector Start = Owner->GetActorLocation();

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WallSlideTrace), false);
	QueryParams.AddIgnoredActor(Owner);

	static constexpr int32 NumDirections = 8;
	for (int32 i = 0; i < NumDirections; ++i)
	{
		const float Angle = (360.f / NumDirections) * i;
		const FVector Dir = FRotator(0.f, Angle, 0.f).RotateVector(FVector::ForwardVector);
		const FVector End = Start + Dir * WallTraceDistance;

		FHitResult Hit;
		const bool bHit = Owner->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams);

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

float UWallSlideAbility::UpdateWallSlide(const float DeltaTime)
{
	const TObjectPtr<ABaseCharacter> Owner = GetTypedOuter<ABaseCharacter>();
	const TObjectPtr<AController>    Controller = Owner->GetController();
	const TObjectPtr<UCharacterMovementComponent> MovementComponent = Owner->GetCharacterMovement();
	
	if (!MovementComponent)
	{
		return 0.0f;
	}

	if (WallJumpCooldownRemaining > 0.f)
	{
		WallJumpCooldownRemaining -= DeltaTime;
	}

	const bool bFalling = MovementComponent->IsFalling();
	const bool bMovingDown = Owner->GetVelocity().Z < 0.f;

	FVector WallNormal;
	const bool bWallDetected = bFalling && bMovingDown && WallJumpCooldownRemaining <= 0.f && DetectWall(WallNormal);

	bIsWallSliding = bWallDetected;

	if (bIsWallSliding)
	{
		CurrentWallNormal = WallNormal;
		MovementComponent->GravityScale = WallSlideGravityScale;
	}
	else
	{
		MovementComponent->GravityScale = 1.6f;
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
	
	return TargetRoll;
}

// 
// DashAbility.h
// 
// Implementation of the 'DashAbility' class.
// 
// ----------------------------------------  x  ---------------------------------------- 
// 
// © 2026 CylindriKill. All rights reserved.
// 

#include "DashAbility.h"

#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "CylindriKill/Character/PlayerCharacter.h"

// ------------------------------------------------------------------
// Constructor & Destructor
// ------------------------------------------------------------------
UDashAbility::UDashAbility()
{
	bIsSliding = false;
	
	ImpulseStrength = 2500.0f;
	
	SlideBrakingDeceleration  = 100.f;	
	SlideBrakingFriction      = 0.0f;
	SlideDuration             = 0.75f;
	SlideGroundFriction       = 0.1f;
	SlideMaxAcceleration      = 500.f;
}

// ------------------------------------------------------------------
// Overridden Methods
// ------------------------------------------------------------------
void UDashAbility::BeginPlay()
{
	Super::BeginPlay();
	
	LastMoveInput = FVector2D::ZeroVector;
	MoveComponent = Parent->GetCharacterMovement();
	
	DefaultGroundFriction = MoveComponent->GroundFriction;
	DefaultBrakingDecelerationWalking = MoveComponent->BrakingDecelerationWalking;
	DefaultBrakingFriction = MoveComponent->BrakingFriction;
	DefaultMaxAcceleration = MoveComponent->MaxAcceleration;
	DefaultGravityScale = MoveComponent->GravityScale;
}

bool UDashAbility::IsActive() const
{
	return bIsSliding;
}

bool UDashAbility::Trigger()
{
	if (!Super::Trigger())
		return false;
	
	const TObjectPtr<AController> Controller = Parent->GetController();
	
	const float    ControlYaw = Controller ? Controller->GetControlRotation().Yaw : Parent->GetActorRotation().Yaw;
	const FRotator YawRotation(0.f, ControlYaw, 0.f);
	const FVector  Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector  Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	FVector SlideDirection = Forward;
	
	if (!LastMoveInput.IsNearlyZero())
		SlideDirection = (Forward * LastMoveInput.Y + Right * LastMoveInput.X).GetSafeNormal();
	
	SlideDirection.Z = 0.f;
	SlideDirection.Normalize();

	const FVector LaunchVelocity(
	   SlideDirection.X * ImpulseStrength,
	   SlideDirection.Y * ImpulseStrength,
	   MoveComponent->Velocity.Z + 200.f);
	
	Parent->LaunchCharacter(LaunchVelocity, true, true);
	
	if (MoveComponent->IsMovingOnGround())
	{
		MoveComponent->GroundFriction = SlideGroundFriction;
		MoveComponent->BrakingDecelerationWalking = SlideBrakingDeceleration;
		MoveComponent->BrakingFriction = SlideBrakingFriction;
		MoveComponent->MaxAcceleration = SlideMaxAcceleration;
	}

	bIsSliding = true;
	
	GetWorld()->GetTimerManager().SetTimer(
	   SlideTimerHandle,
	   this,
	   &UDashAbility::EndSlide,
	   SlideDuration,
	   false);
	
	return true;
}

// ------------------------------------------------------------------
// Internal Methods
// ------------------------------------------------------------------
void UDashAbility::EndSlide()
{
	bIsSliding = false;
	
	MoveComponent->GroundFriction = DefaultGroundFriction;
	MoveComponent->BrakingDecelerationWalking = DefaultBrakingDecelerationWalking;
	MoveComponent->BrakingFriction = DefaultBrakingFriction;
	MoveComponent->MaxAcceleration = DefaultMaxAcceleration;
}

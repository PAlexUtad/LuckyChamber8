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

#include "TimerManager.h"
#include "CylindriKill/BaseCharacter.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"

class ABaseCharacter;
// ------------------------------------------------------------------
// Constructor & Destructor
// ------------------------------------------------------------------
UDashAbility::UDashAbility()
{
	bIsSliding      = false;
	ImpulseStrength = 2500.0f;
	
	SlideBrakingDeceleration  = 100.f;	
	SlideBrakingFriction      = 0.0f;
	SlideDuration             = 0.75f;
	SlideGroundFriction       = 0.1f;
	SlideMaxAcceleration      = 500.f;
}

// ------------------------------------------------------------------
// Exposed Methods
// ------------------------------------------------------------------
bool UDashAbility::Activate()
{
	if (!Super::Activate() || bIsSliding)
		return false;
	
	const TObjectPtr<ABaseCharacter> Owner      = GetTypedOuter<ABaseCharacter>(); 
	const TObjectPtr<AController>    Controller = Owner->GetController();
	
	const float    ControlYaw = Controller ? Controller->GetControlRotation().Yaw : Owner->GetActorRotation().Yaw;
	const FRotator YawRotation(0.f, ControlYaw, 0.f);
	const FVector  Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector  Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	const FVector LaunchVelocity(
	   Forward.X * ImpulseStrength,
	   Forward.Y * ImpulseStrength,
	   Owner->GetCharacterMovement()->Velocity.Z + 200.f);
	
	Owner->LaunchCharacter(LaunchVelocity, true, true);
	
	const TObjectPtr<UCharacterMovementComponent> MovementComponent = Owner->GetCharacterMovement();
	
	if (MovementComponent->IsMovingOnGround())
	{
		GroundFriction  = MovementComponent->GroundFriction;
		BrakingDecelerationWalking = MovementComponent->BrakingDecelerationWalking;
		BrakingFriction = MovementComponent->BrakingFriction;
		MaxAcceleration = MovementComponent->MaxAcceleration;
		GravityScale    = MovementComponent->GravityScale;
		
		MovementComponent->BrakingDecelerationWalking = SlideBrakingDeceleration;
		MovementComponent->BrakingFriction = SlideBrakingFriction;
		MovementComponent->GroundFriction  = SlideGroundFriction;
		MovementComponent->MaxAcceleration = SlideMaxAcceleration;
	}
	
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
	
	const TObjectPtr<ABaseCharacter> Owner = GetTypedOuter<ABaseCharacter>(); 
	const TObjectPtr<UCharacterMovementComponent> MovementComponent = Owner->GetCharacterMovement();
	
	MovementComponent->BrakingDecelerationWalking = BrakingDecelerationWalking;
	MovementComponent->BrakingFriction = BrakingFriction;
	MovementComponent->GroundFriction  = GroundFriction;
	MovementComponent->MaxAcceleration = MaxAcceleration;
}

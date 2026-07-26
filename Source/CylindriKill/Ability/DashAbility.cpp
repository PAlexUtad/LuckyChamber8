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
	if (!Super::Activate())
		return false;
	
	const TObjectPtr<ABaseCharacter> Owner      = GetTypedOuter<ABaseCharacter>(); 
	const TObjectPtr<AController>    Controller = Owner->GetController();
	
	const float    ControlYaw = Controller ? Controller->GetControlRotation().Yaw : Owner->GetActorRotation().Yaw;
	const FRotator YawRotation(0.f, ControlYaw, 0.f);
	const FVector  Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	const FVector LaunchVelocity(
	   Forward.X * ImpulseStrength,
	   Forward.Y * ImpulseStrength,
	   Owner->GetCharacterMovement()->Velocity.Z + 200.f);
	
	Owner->LaunchCharacter(LaunchVelocity, true, true);
	
	const TObjectPtr<UCharacterMovementComponent> MovementComponent = Owner->GetCharacterMovement();
	
	if (MovementComponent->IsMovingOnGround())
	{
		WalkBrakingDeceleration = MovementComponent->BrakingDecelerationWalking;
		WalkBrakingFriction     = MovementComponent->BrakingFriction;
		WalkGroundFriction      = MovementComponent->GroundFriction;
		WalkMaxAcceleration     = MovementComponent->MaxAcceleration;
		
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
void UDashAbility::EndSlide() const
{
	const TObjectPtr<ABaseCharacter> Owner = GetTypedOuter<ABaseCharacter>(); 
	const TObjectPtr<UCharacterMovementComponent> MovementComponent = Owner->GetCharacterMovement();
	
	MovementComponent->BrakingDecelerationWalking = WalkBrakingDeceleration;
	MovementComponent->BrakingFriction            = WalkBrakingFriction;
	MovementComponent->GroundFriction             = WalkGroundFriction;
	MovementComponent->MaxAcceleration            = WalkMaxAcceleration;
}

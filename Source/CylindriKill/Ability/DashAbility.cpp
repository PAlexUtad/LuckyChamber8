
#include "DashAbility.h"

#include "CylindriKill/BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

UDashAbility::UDashAbility()
{	
	CooldownDuration = 0.75f;
	ImpulseStrength  = 2500.0f;
	
	SlideBrakingDeceleration = 100.0f;
	SlideBrakingFriction     = 0.0f;
	SlideCameraDropAmount    = 40.0f;
	SlideCameraInterpSpeed   = -65.0f;
	SlideCameraOffset        = 0.0f;
	SlideDuration            = 0.75f;
	SlideGunInterpSpeed      = 10.0f;
	SlideGunPitchDegree      = -65.0f;
	SlideGunRotationOffset   = FRotator::ZeroRotator;
	SlideGroundFriction      = 0.1f;
	SlideMaxAcceleration     = 500.0f;
	
	if (Parent.Get())
		MovementComponent = Parent->GetCharacterMovement();
}

void UDashAbility::BeginAbility()
{
	Super::BeginAbility();
	
	if (!bCanTrigger || !Parent->GetCharacterMovement())
		return;
	
	const TObjectPtr<AController> Controller = Parent->GetController();
	const FRotator ActorRotation = Parent->GetActorRotation();
	
	const float    ControlYaw = Controller ? Controller->GetControlRotation().Yaw : ActorRotation.Yaw;
	const FRotator YawRotation(0.f, ControlYaw, 0.f);
	const FVector  Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector  Right   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	FVector SlideDirection;
	
	if (!LastMoveInput.IsNearlyZero())
		SlideDirection = (Forward * LastMoveInput.Y + Right * LastMoveInput.X).GetSafeNormal();
	else
		SlideDirection = Forward;
	
	SlideDirection.Z = 0.f;
	SlideDirection.Normalize();
	
	const FVector LaunchVelocity(
	   SlideDirection.X * ImpulseStrength,
	   SlideDirection.Y * ImpulseStrength,
	   MovementComponent->Velocity.Z + 200.f);

	Parent->LaunchCharacter(LaunchVelocity, true, false);
	
	if (MovementComponent->IsMovingOnGround())
	{
		MovementComponent->GroundFriction = SlideGroundFriction;
		MovementComponent->BrakingDecelerationWalking = SlideBrakingDeceleration;
		MovementComponent->BrakingFriction = SlideBrakingFriction;
		MovementComponent->MaxAcceleration = SlideMaxAcceleration;
	} 
	
	bIsSliding = true;
	
	Parent->GetWorldTimerManager().SetTimer(
	   SlideTimerHandle,
	   this,
	   &UDashAbility::EndAbility,
	   SlideDuration,
	   false);
}

void UDashAbility::TickAbility(float DeltaTime)
{
	Super::TickAbility(DeltaTime);
	
	/*
    const float TargetCameraSlideOffset = bIsSliding ? -SlideCameraDropAmount : 0.f;
    CurrentSlideCameraOffset = FMath::FInterpTo(
       CurrentSlideCameraOffset, TargetCameraSlideOffset, DeltaTime, SlideCameraInterpSpeed);

    const FRotator TargetGunSlideRotation = bIsSliding ? FRotator(SlideGunPitchDegrees, 0.f, 0.f) : FRotator::ZeroRotator;
    CurrentGunSlideRotationOffset = FMath::RInterpTo(
       CurrentGunSlideRotationOffset, TargetGunSlideRotation, DeltaTime, SlideGunInterpSpeed);

    if (IsValid(GunChildComponent))
    {
       GunChildComponent->SetRelativeRotation(GunBaseRelativeRotation + CurrentGunSlideRotationOffset);
    }
	*/
}

void UDashAbility::EndAbility()
{
	Super::EndAbility();
	
	bIsSliding = false;
	
	MovementComponent->GroundFriction = 0.0f;
	MovementComponent->BrakingDecelerationWalking = 0.0f;
	MovementComponent->BrakingFriction = 0.0f;
	MovementComponent->MaxAcceleration = 0.0f;
}

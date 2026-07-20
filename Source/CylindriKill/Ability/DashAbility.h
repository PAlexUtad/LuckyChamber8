// 
// DashAbility.h
// 
// Simple dash + slide ability.
// 
// ----------------------------------------  x  ---------------------------------------- 
// 
// © 2026 CylindriKill. All rights reserved.
// 

#pragma once

#include "CylindriKill/BaseAbility.h"
#include "DashAbility.generated.h"

UCLASS()
class CYLINDRIKILL_API UDashAbility : public UBaseAbility
{
	GENERATED_BODY()
	
	// ------------------------------------------------------------------
	// Internal Variables
	// ------------------------------------------------------------------
	bool bIsSliding;
	
	float DefaultGroundFriction;
	float DefaultBrakingDecelerationWalking;
	float DefaultBrakingFriction;
	float DefaultMaxAcceleration;
	float DefaultGravityScale;
	
	FVector2D    LastMoveInput;
	FTimerHandle SlideTimerHandle;
	
	UPROPERTY() TObjectPtr<UCharacterMovementComponent> MoveComponent;
	
	// ------------------------------------------------------------------
	// Blueprint Variables
	// ------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"))
	float ImpulseStrength;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"))
	float SlideBrakingDeceleration;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"))
	float SlideBrakingFriction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"))
	float SlideDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"))
	float SlideGroundFriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"))
	float SlideMaxAcceleration;
	
public:
	
	// ------------------------------------------------------------------
	// Constructor & Destructor
	// ------------------------------------------------------------------
	UDashAbility();
	
	// ------------------------------------------------------------------
	// Overridden Methods
	// ------------------------------------------------------------------
	virtual void BeginPlay() override;
	virtual bool IsActive() const override;
	virtual bool Trigger() override;
	
private:
	
	// ------------------------------------------------------------------
	// Internal Methods
	// ------------------------------------------------------------------
	void EndSlide();
};

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
	// Internal Methods
	// ------------------------------------------------------------------
	bool bIsSliding;
	
	float BrakingDecelerationWalking;
	float BrakingFriction;
	float GravityScale;
	float GroundFriction;
	float MaxAcceleration;
	
	FTimerHandle SlideTimerHandle;

	// ------------------------------------------------------------------
	// Blueprint Variables
	// ------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	float ImpulseStrength;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	float SlideBrakingDeceleration;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	float SlideBrakingFriction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	float SlideDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	float SlideGroundFriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	float SlideMaxAcceleration;
	
public:
	
	// ------------------------------------------------------------------
	// Constructor & Destructor
	// ------------------------------------------------------------------
	UDashAbility();
	
	// ------------------------------------------------------------------
	// Exposed Methods
	// ------------------------------------------------------------------
	virtual bool Activate() override;
	
private:
	
	// ------------------------------------------------------------------
	// Internal Methods
	// ------------------------------------------------------------------
	void EndSlide();
};

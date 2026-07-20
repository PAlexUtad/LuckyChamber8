
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
	bool         bIsSliding;
	float        SlideCameraOffset;
	FRotator     SlideGunRotationOffset;
	FTimerHandle SlideTimerHandle;
	FVector2D    LastMoveInput;
	
	TWeakObjectPtr<UCharacterMovementComponent> MovementComponent;
	
	// ------------------------------------------------------------------
	// Blueprint Variables
	// ------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Dash", Meta = (AllowPrivateAccess = "true"))
	float ImpulseStrength;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Dash", Meta = (AllowPrivateAccess = "true"))
	float SlideBrakingDeceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Dash", Meta = (AllowPrivateAccess = "true"))
	float SlideBrakingFriction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Dash", Meta = (AllowPrivateAccess = "true"))
	float SlideCameraDropAmount;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Dash", Meta = (AllowPrivateAccess = "true"))
	float SlideCameraInterpSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Dash", Meta = (AllowPrivateAccess = "true"))
	float SlideDuration;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Dash", Meta = (AllowPrivateAccess = "true"))
	float SlideGroundFriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Dash", Meta = (AllowPrivateAccess = "true"))
	float SlideGunInterpSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Dash", Meta = (AllowPrivateAccess = "true"))
	float SlideGunPitchDegree;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Dash", Meta = (AllowPrivateAccess = "true"))
	float SlideMaxAcceleration;
	
public:
	
	// ------------------------------------------------------------------
	// Constructor & Destructor
	// ------------------------------------------------------------------
	UDashAbility();
	
	// ------------------------------------------------------------------
	// Overridden Methods
	// ------------------------------------------------------------------
	virtual void BeginAbility() override;
	virtual void TickAbility(float) override;
	virtual void EndAbility() override;
};

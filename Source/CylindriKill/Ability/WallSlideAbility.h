// 
// WallSlideAbility.h
// 
// Wall running.
// 
// ----------------------------------------  x  ---------------------------------------- 
// 
// © 2026 CylindriKill. All rights reserved.
// 

#pragma once

#include "CylindriKill/BaseAbility.h"
#include "WallSlideAbility.generated.h"

UCLASS()
class CYLINDRIKILL_API UWallSlideAbility : public UBaseAbility
{
	GENERATED_BODY()
	
	bool    bIsWallSliding;
	FVector CurrentWallNormal;
	float   WallJumpCooldownRemaining;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	bool bDrawDebugWallTrace;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	float WallJumpAwayStrength;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	float WallJumpReattachCooldown;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	float WallJumpUpStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	float WallSlideCameraRollDegrees;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	float WallSlideCameraRollInterpSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	float WallSlideGravityScale;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	float WallTraceDistance;
	
public:
	
	// ------------------------------------------------------------------
	// Constructor & Destructor
	// ------------------------------------------------------------------
	UWallSlideAbility();
	
	// ------------------------------------------------------------------
	// Exposed Methods
	// ------------------------------------------------------------------
	virtual bool Activate() override;
	virtual bool IsInProgress() const override;
	bool DetectWall(FVector& OutWallNormal) const;
	float UpdateWallSlide(const float DeltaTime);
	
	FORCEINLINE float GetWallSlideCameraRollInterpSpeed() const { return WallSlideCameraRollInterpSpeed; }
};

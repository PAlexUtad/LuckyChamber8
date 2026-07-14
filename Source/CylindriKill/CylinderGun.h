// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGun.h"
#include "CylinderGun.generated.h"

/**
 * Gun variant whose body is a cube mesh with a cylinder mounted on top,
 * both loaded directly from disk in the constructor.
 */
UCLASS()
class CYLINDRIKILL_API ACylinderGun : public ABaseGun
{
	GENERATED_BODY()

public:
	ACylinderGun();

protected:
	/** Extra mesh piece attached to the inherited BodyMesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> CylinderMesh;

	//~ Begin ABaseGun Interface
	/** Runs the inherited hitscan fire logic, then spins the cylinder a notch for a revolver feel. */
	virtual void Fire() override;
	//~ End ABaseGun Interface

	/** Degrees the cylinder rotates per shot (6 chambers -> 60 degrees is the classic revolver step). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Cylinder")
	float CylinderStepDegrees = 60.f;
};

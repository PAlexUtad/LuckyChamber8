// Fill out your copyright notice in the Description page of Project Settings.

#include "CylinderGun.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ACylinderGun::ACylinderGun()
{
	// -------------------------------------------------------------
	// Body mesh (inherited from ABaseGun) - assign the cube asset.
	// ConstructorHelpers::FObjectFinder only works inside a constructor
	// and only with a literal path string.
	// -------------------------------------------------------------
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BodyMeshAsset(TEXT("/Game/Meshes/Gun/Cube.Cube"));
	if (BodyMeshAsset.Succeeded())
	{
		BodyMesh->SetStaticMesh(BodyMeshAsset.Object);
	}

	// -------------------------------------------------------------
	// Cylinder mesh - new component, attached to the main body so it
	// moves/rotates with it.
	// -------------------------------------------------------------
	CylinderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CylinderMesh"));
	CylinderMesh->SetupAttachment(BodyMesh);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshAsset(TEXT("/Game/Meshes/Gun/Cylinder.Cylinder"));
	if (CylinderMeshAsset.Succeeded())
	{
		CylinderMesh->SetStaticMesh(CylinderMeshAsset.Object);
	}
}

void ACylinderGun::Fire()
{
	// Cache ammo before firing so we only spin the cylinder when a shot actually goes off
	// (Super::Fire() silently no-ops on cooldown/empty mag).
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("ACylinderGun StartFIre!"));
	const int32 AmmoBeforeFire = CurrentAmmo;

	Super::Fire();

	if (CurrentAmmo < AmmoBeforeFire && CylinderMesh)
	{
		// Purely cosmetic revolver "chk-chk" spin - rotate the cylinder one chamber per shot.
		CylinderMesh->AddLocalRotation(FRotator(0.f, 0.f, CylinderStepDegrees));
	}
}

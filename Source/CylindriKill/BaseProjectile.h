#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UPaperSpriteComponent;

UCLASS()
class CYLINDRIKILL_API ABaseProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseProjectile();

protected:
	virtual void BeginPlay() override;

	/** Collision sphere for overlap detection */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	/** Visual sprite for the projectile */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> SpriteComponent;

	/** Handles movement velocity and physics */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UFUNCTION()
	void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult
	);
};
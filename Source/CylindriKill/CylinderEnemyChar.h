#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Plugins/2D/Paper2D/Source/Paper2D/Classes/PaperSpriteComponent.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "CylinderEnemyChar.generated.h"


UCLASS()
class CYLINDRIKILL_API ACylinderEnemyChar : public ACharacter
{
	GENERATED_BODY()

public:
	ACylinderEnemyChar();

	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPaperSpriteComponent> SpriteComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Health = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	bool bFacePlayer = true;
	
	/** Projectile class to spawn when firing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<class ABaseProjectile> ProjectileClass;

	FTimerHandle FireTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float ShootInterval = 2.0f;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void FireAtPlayer();

	/** Hook for subclasses to add custom movement each tick, called after facing logic */
	virtual void UpdateMovement(float DeltaTime) {}

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MoveToLocationViaNav(const FVector& TargetLocation, float AcceptanceRadius = 50.f);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MoveToActorViaNav(AActor* TargetActor, float AcceptanceRadius = 50.f);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopNavMovement();
private:
	void RotateToFacePlayer(float DeltaTime);
};
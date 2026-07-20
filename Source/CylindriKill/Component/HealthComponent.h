// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChanged, float, NewHealth, float, MaxHealth, float, DamageAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, DamageCauser);

/**
 * UHealthComponent
 *
 * Drop this on any actor (player, enemy, future destructibles) to make it damageable/killable.
 * Damage-dealing code (guns, projectiles) never needs to know the concrete class of what it hit -
 * it just calls UHealthComponent::FindHealthComponent(HitActor) and, if one exists, calls ApplyDamage
 * on it. Death is broadcast via a delegate so each owning actor decides what "dying" actually means
 * (Destroy(), ragdoll, respawn, etc.) without HealthComponent needing to know either.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CYLINDRIKILL_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHealth = 100.f;

	/** If true, damage is still tracked (health drops, OnHealthChanged fires) but death never triggers. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	bool bIsInvincible = false;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeath OnDeath;

	/** Applies damage, clamps health, fires OnHealthChanged, and fires OnDeath once when health first reaches 0. Returns actual damage applied (may be less than requested if it would've gone below 0). */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float ApplyDamage(float DamageAmount, AActor* DamageCauser = nullptr, AController* Instigator = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float HealAmount);

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const { return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f; }

	/** Convenience static helper used by damage sources (guns, projectiles) to generically check
	 *  "can this actor be damaged/killed" without needing to know its concrete class at all. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	static UHealthComponent* FindHealthComponent(AActor* Actor);

protected:
	virtual void BeginPlay() override;

private:
	bool bIsDead = false;
};
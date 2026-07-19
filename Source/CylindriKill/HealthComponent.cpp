// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // purely event/function driven, no per-frame work needed
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	bIsDead = false;
}

float UHealthComponent::ApplyDamage(float DamageAmount, AActor* DamageCauser, AController* Instigator)
{
	if (bIsDead || DamageAmount <= 0.f)
	{
		return 0.f;
	}

	const float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.f, MaxHealth);
	const float ActualDamage = OldHealth - CurrentHealth;

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, ActualDamage);

	if (CurrentHealth <= 0.f && !bIsInvincible)
	{
		bIsDead = true;
		OnDeath.Broadcast(DamageCauser);
	}

	return ActualDamage;
}

void UHealthComponent::Heal(float HealAmount)
{
	if (bIsDead || HealAmount <= 0.f)
	{
		return;
	}

	const float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, CurrentHealth - OldHealth);
}

UHealthComponent* UHealthComponent::FindHealthComponent(AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UHealthComponent>() : nullptr;
}
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "HitWidgetComponent.generated.h"

class UFloatingTextWidget;

/**
 * 
 */
UCLASS()
class CYLINDRIKILL_API UHitWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

	UPROPERTY()
	UFloatingTextWidget* DamageWidget;

	UPROPERTY(EditDefaultsOnly, category = "Damage UI")
	TSubclassOf<UFloatingTextWidget> DamageWidgetClass;

public:
	UHitWidgetComponent();
	virtual void BeginPlay() override;

	// Function to trigger damage text from outside
	UFUNCTION(BlueprintCallable, category = "Pooling|Damage")
	void SpawnDamageText(float _, float MaxHealth, float DamageAmount);
};

// 
// BaseCharacter.h
// 
// Parent class for all characters used in the game. 
// 
// ----------------------------------------  x  ---------------------------------------- 
// 
// © 2026 CylindriKill. All rights reserved.
// 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class UBaseAbility;
class UPaperSpriteComponent;
class UHealthComponent;

UCLASS(CollapseCategories)
class CYLINDRIKILL_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()
	
protected:
	
	// ------------------------------------------------------------------
	// Blueprint Variables
	// ------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", Instanced, meta = (AllowPrivateAccess = "true")) // <- Added comma before meta
	TArray<TObjectPtr<UBaseAbility>> Abilities;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;
	
	// TODO: Remove this field once a default mesh has been implemented.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPaperSpriteComponent> SpriteComponent;
	
public:

	// ------------------------------------------------------------------
	// Constructor & Destructor
	// ------------------------------------------------------------------
	ABaseCharacter();
	
	// ------------------------------------------------------------------
	// Overridden Methods
	// ------------------------------------------------------------------
	virtual void BeginPlay() override;
	virtual void Tick(const float) override;
	
protected:
	
	// ------------------------------------------------------------------
	// Blueprintable Methods
	// ------------------------------------------------------------------
	UFUNCTION(BlueprintCallable) virtual void Die(AActor* Aggressor);
	UFUNCTION(BlueprintCallable) virtual void ActivateAbility(const TSubclassOf<UBaseAbility> AbilityClass);
	UFUNCTION(BlueprintCallable) virtual UBaseAbility* FindAbility(const TSubclassOf<UBaseAbility> AbilityClass);
};

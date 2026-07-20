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

class UPaperSpriteComponent;
class UHealthComponent;

UCLASS()
class CYLINDRIKILL_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	
	// ------------------------------------------------------------------
	// Blueprint Variables
	// ------------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;
	
	// TODO: Remove this field once a default mesh has been implemented.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPaperSpriteComponent> SpriteComponent;
	
public:

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
	UFUNCTION() virtual void Spawn();
	UFUNCTION() virtual void Die(AActor* Aggressor);
};

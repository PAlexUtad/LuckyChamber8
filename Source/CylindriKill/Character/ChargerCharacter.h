
#pragma once

#include "CoreMinimal.h"
#include "CylindriKill/CylinderEnemyChar.h"
#include "ChargerCharacter.generated.h"

UCLASS()
class CYLINDRIKILL_API AChargerCharacter : public ACylinderEnemyChar
{
	GENERATED_BODY()
public:
	AChargerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void UpdateMovement(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge")
	float TriggerRange = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge")
	float TelegraphTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge")
	float ChargeSpeed = 1200.f;

	/** Once within this range, stop charging and start unloading shots */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge")
	float FireRange = 350.f;

	/** Fire rate once planted and shooting (faster than the base's default ShootInterval) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge")
	float CloseRangeShootInterval = 0.3f;

private:
	bool bIsCharging = false;
	bool bIsShooting = false;
	float TelegraphRemaining = 0.f;
	float NormalWalkSpeed = 0.f;
};
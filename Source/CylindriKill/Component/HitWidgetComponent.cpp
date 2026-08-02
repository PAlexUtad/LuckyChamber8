// Fill out your copyright notice in the Description page of Project Settings.


#include "HitWidgetComponent.h"
#include "CylindriKill/UI/HUD/FloatingTextWidget.h"
#include "GameFramework/Character.h"
#include <Kismet/GameplayStatics.h>
#include <Kismet/KismetMathLibrary.h>

UHitWidgetComponent::UHitWidgetComponent() {

	Space = EWidgetSpace::World;
	TickMode = ETickMode::Enabled; 
	
	bHiddenInGame = false; 
	bIsTwoSided = true;
	TranslucencySortPriority = 1;

	SetVisibility(true);
	SetDrawSize(FVector2D(200.0f, 300.0f));

	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);

	static ConstructorHelpers::FClassFinder<UFloatingTextWidget> DamageTextWidgetAsset(TEXT("WidgetBlueprint'/Game/Blueprints/UI/HUD/WBP_FloatingText_Damage.WBP_FloatingText_Damage_C'"));
	if (DamageTextWidgetAsset.Succeeded())
	{
		DamageWidgetClass = DamageTextWidgetAsset.Class;
	}
}

void UHitWidgetComponent::BeginPlay() {

	if (DamageWidgetClass)
	{
		DamageWidget = CreateWidget<UFloatingTextWidget>(GetWorld(), DamageWidgetClass);
		
		if (DamageWidget)
		{
			SetWidget(DamageWidget);
			InitWidget();
			SetVisibility(true);
		}
	}
}

void UHitWidgetComponent::SpawnDamageText(float _, float MaxHealth, float DamageAmount) {
	
	if (!DamageWidget) { return; }

	DamageWidget->ActivateText(
		FText::FromString(FString::SanitizeFloat(DamageAmount)), 
		DamageAmount / MaxHealth > 0.3f ? FLinearColor::Red : FLinearColor::Blue
	);

	// Rotate towards player
	if (APawn* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(
			GetComponentLocation(),
			PlayerCharacter->GetActorLocation()
		);
		SetWorldRotation(NewRotation);
	}
}

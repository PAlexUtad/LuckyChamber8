#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimationEvents.h"
#include "FloatingTextWidget.generated.h"

// TODO: Move this to utils/tweening
UENUM(BlueprintType)
enum class EEasingFunction : uint8
{
	None = 0	UMETA(DisplayName = "No Tweening"),
	Linear		UMETA(DisplayName = "Linear", ToolTip = "Defaults to Linear Interpolation"),
	Sine		UMETA(DisplayName = "Sine"),
	Quad		UMETA(DisplayName = "Quad"),
	Cubic		UMETA(DisplayName = "Cubic"),
	Quart		UMETA(DisplayName = "Quart"),
	Quint		UMETA(DisplayName = "Quint"),
	Exponential	UMETA(DisplayName = "Exponential"),
	Circular	UMETA(DisplayName = "Circular", ToolTip = "For ease out, this follows the curve the top left side of a circle"),
	Back		UMETA(DisplayName = "Back", ToolTip = "Starts by going back before going forward."),
	Elastic		UMETA(DisplayName = "Elastic", ToolTip = "On ease out, vibrates when reaching the end, like a guitar string."),
	Bounce		UMETA(DisplayName = "Bounce", ToolTip = "On ease out, bounces back multiple time when reaching the end, like a dropped ball."),
};

// TODO: Move this to utils/tweening
UENUM(BlueprintType)
enum class EasingType : uint8 {
	None = 0	UMETA(Hidden),
	EaseIn		UMETA(DisplayName = "Ease-In", ToolTip = "Tweening function applies to the begining of the cruve."),
	EaseOut		UMETA(DisplayName = "Ease-Out", ToolTip = "Tweening function applies to the end of the cruve."),
	EaseInOut	UMETA(DisplayName = "Ease-In-Out", ToolTip = "Tweening function applies to both points of the cruve."),
};


// Forward declarations
class UTextBlock;
class UWidgetAnimation;

/**
 *
 */
UCLASS()
class CYLINDRIKILL_API UFloatingTextWidget : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Animation")
	float AnimationTime;

	UPROPERTY(EditAnywhere, Category = "Animation", meta = (ClampMin = 0.01f, ClampMax = 0.5f, ToolTip = "Minimum size in relation to starting size."))
	float EndSize;

	UPROPERTY(EditAnywhere, Category = "Animation|Tweening")
	EEasingFunction TweenFnc;

	UPROPERTY(EditAnywhere, Category = "Animation|Tweening", meta = (EditCondition = "TweenFnc != EEasingFunction::None && TweenFnc != EEasingFunction::Linear"))
	EasingType TweenType;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DamageText;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOutAnim;

	UPROPERTY()
	FWidgetAnimationDynamicEvent OnFadeOutEnd;

public:
	UFUNCTION(BlueprintCallable)
	void ActivateText(const FText& Text, const FLinearColor& Color);

private:
	UFUNCTION()
	void FadeOutEnd();

	virtual void NativeConstruct() override;
};
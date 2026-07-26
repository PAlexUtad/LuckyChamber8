#include "FloatingTextWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"

void UFloatingTextWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UUserWidget::SetVisibility(ESlateVisibility::Collapsed);

	AnimationTime = 0.75f;
	EndSize = 0.35f;
	TweenFnc = EEasingFunction::None;
	TweenType = EasingType::None;
}

void UFloatingTextWidget::FadeOutEnd()
{
	UUserWidget::SetVisibility(ESlateVisibility::Collapsed);
}

void UFloatingTextWidget::ActivateText(const FText& Text, const FLinearColor& Color)
{
	// Bind safely here because NativeConstruct and layout parsing 
	// are finished (otherwise may be compiled away).
	if (FadeOutAnim && !OnFadeOutEnd.IsBound())
	{
		OnFadeOutEnd.BindDynamic(this, &UFloatingTextWidget::FadeOutEnd);
		UUserWidget::BindToAnimationFinished(FadeOutAnim, OnFadeOutEnd);
	}

	DamageText->SetText(Text);
	DamageText->SetColorAndOpacity(FSlateColor(Color));
	UUserWidget::SetVisibility(ESlateVisibility::Visible);
	PlayAnimation(FadeOutAnim);
}
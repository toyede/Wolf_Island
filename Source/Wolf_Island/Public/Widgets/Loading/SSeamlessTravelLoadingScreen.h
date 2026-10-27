// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FDeferredCleanupSlateBrush;
class SImage;
struct FALoadingScreenSettings;
struct FClassicLayoutSettings;
struct FLoadingWidgetSettings;
struct FTipSettings;

//JWY-Seamless Travel(로비->게임)에서는 Async Loading Screen 플러그인이 뜨지 않아,
//플러그인의 Default Loading Screen 설정을 그대로 읽어 같은 디자인(Classic 레이아웃)으로 그리는 Slate 위젯.
//플러그인의 Slate 위젯 클래스는 export되지 않아 직접 생성할 수 없으므로 같은 구성을 이 위젯에서 다시 만든다.
class WOLF_ISLAND_API SSeamlessTravelLoadingScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSeamlessTravelLoadingScreen) {}
	SLATE_END_ARGS()

	//Settings/LayoutSettings는 ULoadingScreenSettings CDO의 값을 넘겨야 한다(브러시 포인터를 그대로 참조함)
	void Construct(const FArguments& InArgs, const FALoadingScreenSettings& Settings, const FClassicLayoutSettings& LayoutSettings);

private:
	TSharedRef<SWidget> ConstructBackground(const FALoadingScreenSettings& Settings);
	TSharedRef<SWidget> ConstructLoadingWidget(const FLoadingWidgetSettings& Settings);
	TSharedRef<SWidget> ConstructLoadingIcon(const FLoadingWidgetSettings& Settings);
	TSharedRef<SWidget> ConstructLoadingText(const FLoadingWidgetSettings& Settings) const;
	TSharedRef<SWidget> ConstructTip(const FTipSettings& Settings) const;

	EActiveTimerReturnType AdvanceImageSequence(double InCurrentTime, float InDeltaTime);

	TSharedPtr<FDeferredCleanupSlateBrush> BackgroundBrush;
	TArray<TSharedPtr<FDeferredCleanupSlateBrush>> ImageSequenceBrushes;
	TSharedPtr<SImage> ImageSequenceIcon;
	int32 ImageSequenceIndex = 0;
	bool bPlayImageSequenceReverse = false;
};

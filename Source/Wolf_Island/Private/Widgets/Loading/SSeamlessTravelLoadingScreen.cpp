// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Loading/SSeamlessTravelLoadingScreen.h"

#include "AsyncLoadingScreenLibrary.h"
#include "Engine/Texture2D.h"
#include "LoadingScreenSettings.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSafeZone.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

void SSeamlessTravelLoadingScreen::Construct(const FArguments& InArgs, const FALoadingScreenSettings& Settings, const FClassicLayoutSettings& LayoutSettings)
{
	//JWY-플러그인 SClassicLayout과 같은 구성: 배경 위에 [팁 | 로딩 위젯] 테두리를 위/아래에 배치
	//게임 뷰포트에 붙으므로 DPI 스케일은 뷰포트가 처리한다(플러그인처럼 SDPIScaler를 따로 두지 않음)
	const TSharedRef<SWidget> LoadingWidget = ConstructLoadingWidget(Settings.LoadingWidget);
	const TSharedRef<SWidget> TipWidget = ConstructTip(Settings.TipWidget);

	TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);

	if (LayoutSettings.bIsLoadingWidgetAtLeft)
	{
		HorizontalBox->AddSlot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.AutoWidth()
			[
				LoadingWidget
			];

		HorizontalBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoWidth()
			[
				SNew(SSpacer)
				.Size(FVector2D(LayoutSettings.Space, 0.0f))
			];

		HorizontalBox->AddSlot()
			.FillWidth(1.0f)
			.HAlign(LayoutSettings.TipAlignment.HorizontalAlignment)
			.VAlign(LayoutSettings.TipAlignment.VerticalAlignment)
			[
				TipWidget
			];
	}
	else
	{
		HorizontalBox->AddSlot()
			.FillWidth(1.0f)
			.HAlign(LayoutSettings.TipAlignment.HorizontalAlignment)
			.VAlign(LayoutSettings.TipAlignment.VerticalAlignment)
			[
				TipWidget
			];

		HorizontalBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoWidth()
			[
				SNew(SSpacer)
				.Size(FVector2D(LayoutSettings.Space, 0.0f))
			];

		HorizontalBox->AddSlot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.AutoWidth()
			[
				LoadingWidget
			];
	}

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			ConstructBackground(Settings)
		]
		+ SOverlay::Slot()
		.HAlign(LayoutSettings.BorderHorizontalAlignment)
		.VAlign(LayoutSettings.bIsWidgetAtBottom ? VAlign_Bottom : VAlign_Top)
		[
			SNew(SBorder)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.BorderImage(&LayoutSettings.BorderBackground)
			.BorderBackgroundColor(FLinearColor::White)
			[
				SNew(SSafeZone)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.IsTitleSafe(true)
				.Padding(LayoutSettings.BorderPadding)
				[
					HorizontalBox
				]
			]
		]
	];
}

TSharedRef<SWidget> SSeamlessTravelLoadingScreen::ConstructBackground(const FALoadingScreenSettings& Settings)
{
	const FBackgroundSettings& Background = Settings.Background;
	if (Background.Images.Num() == 0)
	{
		return SNullWidget::NullWidget;
	}

	int32 ImageIndex = FMath::RandRange(0, Background.Images.Num() - 1);
	if (Background.bSetDisplayBackgroundManually && Background.Images.IsValidIndex(UAsyncLoadingScreenLibrary::GetDisplayBackgroundIndex()))
	{
		ImageIndex = UAsyncLoadingScreenLibrary::GetDisplayBackgroundIndex();
	}

	UTexture2D* BackgroundTexture = Cast<UTexture2D>(Background.Images[ImageIndex].TryLoad());
	if (!BackgroundTexture)
	{
		return SNullWidget::NullWidget;
	}

	//JWY-브러시가 텍스처를 GC 참조로 붙잡아 Seamless Travel 도중 GC에도 배경이 유지된다
	BackgroundBrush = FDeferredCleanupSlateBrush::CreateBrush(BackgroundTexture);

	return SNew(SBorder)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(Background.Padding)
		.BorderBackgroundColor(Background.BackgroundColor)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		[
			SNew(SScaleBox)
			.Stretch(Background.ImageStretch)
			[
				SNew(SImage)
				.Image(FDeferredCleanupSlateBrush::TrySlateBrush(BackgroundBrush))
			]
		];
}

TSharedRef<SWidget> SSeamlessTravelLoadingScreen::ConstructLoadingWidget(const FLoadingWidgetSettings& Settings)
{
	const TSharedRef<SWidget> LoadingIcon = ConstructLoadingIcon(Settings);
	const TSharedRef<SWidget> LoadingText = ConstructLoadingText(Settings);

	if (Settings.LoadingWidgetType == ELoadingWidgetType::LWT_Horizontal)
	{
		TSharedRef<SHorizontalBox> Root = SNew(SHorizontalBox);
		const TSharedRef<SWidget> First = Settings.bLoadingTextRightPosition ? LoadingIcon : LoadingText;
		const TSharedRef<SWidget> Second = Settings.bLoadingTextRightPosition ? LoadingText : LoadingIcon;
		const FWidgetAlignment& FirstAlignment = Settings.bLoadingTextRightPosition ? Settings.LoadingIconAlignment : Settings.TextAlignment;
		const FWidgetAlignment& SecondAlignment = Settings.bLoadingTextRightPosition ? Settings.TextAlignment : Settings.LoadingIconAlignment;

		Root->AddSlot()
			.HAlign(FirstAlignment.HorizontalAlignment)
			.VAlign(FirstAlignment.VerticalAlignment)
			.AutoWidth()
			[
				First
			];

		Root->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoWidth()
			[
				SNew(SSpacer)
				.Size(FVector2D(Settings.Space, 0.0f))
			];

		Root->AddSlot()
			.HAlign(SecondAlignment.HorizontalAlignment)
			.VAlign(SecondAlignment.VerticalAlignment)
			.AutoWidth()
			[
				Second
			];

		return Root;
	}

	TSharedRef<SVerticalBox> Root = SNew(SVerticalBox);
	const TSharedRef<SWidget> First = Settings.bLoadingTextTopPosition ? LoadingText : LoadingIcon;
	const TSharedRef<SWidget> Second = Settings.bLoadingTextTopPosition ? LoadingIcon : LoadingText;
	const FWidgetAlignment& FirstAlignment = Settings.bLoadingTextTopPosition ? Settings.TextAlignment : Settings.LoadingIconAlignment;
	const FWidgetAlignment& SecondAlignment = Settings.bLoadingTextTopPosition ? Settings.LoadingIconAlignment : Settings.TextAlignment;

	Root->AddSlot()
		.HAlign(FirstAlignment.HorizontalAlignment)
		.VAlign(FirstAlignment.VerticalAlignment)
		.AutoHeight()
		[
			First
		];

	Root->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.AutoHeight()
		[
			SNew(SSpacer)
			.Size(FVector2D(0.0f, Settings.Space))
		];

	Root->AddSlot()
		.HAlign(SecondAlignment.HorizontalAlignment)
		.VAlign(SecondAlignment.VerticalAlignment)
		.AutoHeight()
		[
			Second
		];

	return Root;
}

TSharedRef<SWidget> SSeamlessTravelLoadingScreen::ConstructLoadingIcon(const FLoadingWidgetSettings& Settings)
{
	TSharedRef<SWidget> LoadingIcon = SNullWidget::NullWidget;

	if (Settings.LoadingIconType == ELoadingIconType::LIT_ImageSequence)
	{
		const FImageSequenceSettings& Sequence = Settings.ImageSequenceSettings;
		for (UTexture2D* Image : Sequence.Images)
		{
			if (Image)
			{
				ImageSequenceBrushes.Add(FDeferredCleanupSlateBrush::CreateBrush(
					Image,
					FVector2D(Image->GetSurfaceWidth() * Sequence.Scale.X, Image->GetSurfaceHeight() * Sequence.Scale.Y)));
			}
		}

		if (ImageSequenceBrushes.Num() == 0)
		{
			return SNew(SSpacer).Size(FVector2D::ZeroVector);
		}

		ImageSequenceIndex = 0;
		bPlayImageSequenceReverse = Sequence.bPlayReverse;
		ImageSequenceIcon = SNew(SImage)
			.Image(FDeferredCleanupSlateBrush::TrySlateBrush(ImageSequenceBrushes[ImageSequenceIndex]));
		LoadingIcon = ImageSequenceIcon.ToSharedRef();

		if (ImageSequenceBrushes.Num() > 1)
		{
			//JWY-플러그인은 OnPaint에서 프레임을 넘기지만, 뷰포트 위젯은 Active Timer로 넘겨야 화면이 계속 갱신된다
			RegisterActiveTimer(Sequence.Interval, FWidgetActiveTimerDelegate::CreateSP(this, &SSeamlessTravelLoadingScreen::AdvanceImageSequence));
		}
	}
	else if (Settings.LoadingIconType == ELoadingIconType::LIT_CircularThrobber)
	{
		LoadingIcon = SNew(SCircularThrobber)
			.NumPieces(Settings.CircularThrobberSettings.NumberOfPieces)
			.Period(Settings.CircularThrobberSettings.Period)
			.Radius(Settings.CircularThrobberSettings.Radius)
			.PieceImage(&Settings.CircularThrobberSettings.Image);
	}
	else
	{
		const FThrobberSettings& Throbber = Settings.ThrobberSettings;
		const int32 AnimationParams = (Throbber.bAnimateVertically ? SThrobber::Vertical : 0)
			| (Throbber.bAnimateHorizontally ? SThrobber::Horizontal : 0)
			| (Throbber.bAnimateOpacity ? SThrobber::Opacity : 0);

		LoadingIcon = SNew(SThrobber)
			.NumPieces(Throbber.NumberOfPieces)
			.Animate(static_cast<SThrobber::EAnimation>(AnimationParams))
			.PieceImage(&Throbber.Image);
	}

	LoadingIcon->SetRenderTransform(FSlateRenderTransform(FScale2D(Settings.TransformScale), Settings.TransformTranslation));
	LoadingIcon->SetRenderTransformPivot(Settings.TransformPivot);

	return LoadingIcon;
}

TSharedRef<SWidget> SSeamlessTravelLoadingScreen::ConstructLoadingText(const FLoadingWidgetSettings& Settings) const
{
	return SNew(STextBlock)
		.Visibility(Settings.LoadingText.IsEmpty() ? EVisibility::Collapsed : EVisibility::SelfHitTestInvisible)
		.ColorAndOpacity(Settings.Appearance.ColorAndOpacity)
		.Font(Settings.Appearance.Font)
		.ShadowOffset(Settings.Appearance.ShadowOffset)
		.ShadowColorAndOpacity(Settings.Appearance.ShadowColorAndOpacity)
		.Justification(Settings.Appearance.Justification)
		.Text(Settings.LoadingText);
}

TSharedRef<SWidget> SSeamlessTravelLoadingScreen::ConstructTip(const FTipSettings& Settings) const
{
	if (Settings.TipText.Num() == 0)
	{
		return SNullWidget::NullWidget;
	}

	int32 TipIndex = FMath::RandRange(0, Settings.TipText.Num() - 1);
	if (Settings.bSetDisplayTipTextManually && Settings.TipText.IsValidIndex(UAsyncLoadingScreenLibrary::GetDisplayTipTextIndex()))
	{
		TipIndex = UAsyncLoadingScreenLibrary::GetDisplayTipTextIndex();
	}

	return SNew(STextBlock)
		.ColorAndOpacity(Settings.Appearance.ColorAndOpacity)
		.Font(Settings.Appearance.Font)
		.ShadowOffset(Settings.Appearance.ShadowOffset)
		.ShadowColorAndOpacity(Settings.Appearance.ShadowColorAndOpacity)
		.Justification(Settings.Appearance.Justification)
		.WrapTextAt(Settings.TipWrapAt)
		.Text(Settings.TipText[TipIndex]);
}

EActiveTimerReturnType SSeamlessTravelLoadingScreen::AdvanceImageSequence(double InCurrentTime, float InDeltaTime)
{
	if (!ImageSequenceIcon.IsValid() || ImageSequenceBrushes.Num() == 0)
	{
		return EActiveTimerReturnType::Stop;
	}

	ImageSequenceIndex += bPlayImageSequenceReverse ? -1 : 1;
	if (ImageSequenceIndex >= ImageSequenceBrushes.Num())
	{
		ImageSequenceIndex = 0;
	}
	else if (ImageSequenceIndex < 0)
	{
		ImageSequenceIndex = ImageSequenceBrushes.Num() - 1;
	}

	ImageSequenceIcon->SetImage(FDeferredCleanupSlateBrush::TrySlateBrush(ImageSequenceBrushes[ImageSequenceIndex]));
	return EActiveTimerReturnType::Continue;
}

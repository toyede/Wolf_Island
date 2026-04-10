// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/IconButton.h"

#include "IAutomationReport.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"

void UIconButton::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	BackGround->SetBrushColor(ButtonColor);
	
	if (ButtonPadding.Bottom > 0.0f || ButtonPadding.Right > 0.0f || ButtonPadding.Left > 0.0f || ButtonPadding.Right > 0.0f)
	{
		FButtonStyle Style = Button->GetStyle();
		Style.SetNormalPadding(ButtonPadding);
		Style.SetPressedPadding(ButtonPadding);

		Button->SetStyle(Style);
	}
	
	Icon->SetBrush(ImageBrush);
	
	if (ButtonSize.X > 0.0f)
		SizeBox->SetWidthOverride(ButtonSize.X);
	
	if ( ButtonSize.Y > 0.0f)
		SizeBox->SetHeightOverride(ButtonSize.Y);
}

void UIconButton::NativeConstruct()
{
	Super::NativeConstruct();
	
	Button->OnClicked.AddDynamic(this, &UIconButton::OnButtonClicked);
}

void UIconButton::OnButtonClicked()
{
	OnClicked.Broadcast();
}

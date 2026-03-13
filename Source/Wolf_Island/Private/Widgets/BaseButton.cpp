// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/BaseButton.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

void UBaseButton::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	Text->SetText(TextContent);
	
	if (Font.FontObject)
	{
		Text->SetFont(Font);
	}
	
	BackGround->SetBrushColor(ButtonColor);
	
	if (ButtonPadding.Bottom > 0.0f || ButtonPadding.Right > 0.0f || ButtonPadding.Left > 0.0f || ButtonPadding.Right > 0.0f)
	{
		FButtonStyle Style = Button->GetStyle();
		Style.SetNormalPadding(ButtonPadding);
		Style.SetPressedPadding(ButtonPadding);

		Button->SetStyle(Style);
	}
	
	if (ButtonSize.X > 0.0f)
		SizeBox->SetWidthOverride(ButtonSize.X);
	
	if ( ButtonSize.Y > 0.0f)
		SizeBox->SetHeightOverride(ButtonSize.Y);
}

void UBaseButton::NativeConstruct()
{
	Super::NativeConstruct();
	
	Button->OnClicked.AddDynamic(this, &UBaseButton::OnButtonClicked);
}

void UBaseButton::OnButtonClicked()
{
	OnClicked.Broadcast();
}

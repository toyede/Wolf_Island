// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/TextButton.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UTextButton::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (Button)
	{
		Button->OnClicked.AddDynamic(this, &UTextButton::OnClick);
	}
	
	Text->SetText(TextContent);
	
	if (DefaultFont.FontObject)
	{
		Text->SetFont(DefaultFont);
	}
}

void UTextButton::SetSelected(bool IsSelected)
{
	if (IsSelected)
	{
		if (SelectedFont.FontObject)
		{
			Text->SetFont(SelectedFont);
		}
		Text->SetColorAndOpacity(SelectedTextColor);
	} else
	{
		if (DefaultFont.FontObject)
		{
			Text->SetFont(DefaultFont);
		}
		Text->SetColorAndOpacity(DefaultTextColor);
	}
}

void UTextButton::OnClick()
{
	OnClicked.Broadcast(this);
}

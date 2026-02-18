// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ExitButton.h"

#include "Components/Button.h"

void UExitButton::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UExitButton::OnExit);
	}
}

void UExitButton::OnExit()
{
	if (UUserWidget* OwnerWidget = GetTypedOuter<UUserWidget>())
	{
		OwnerWidget->RemoveFromParent();
	}
}
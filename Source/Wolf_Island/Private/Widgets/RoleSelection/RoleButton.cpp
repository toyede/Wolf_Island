// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/RoleSelection/RoleButton.h"

#include "Components/Button.h"

void URoleButton::NativeConstruct()
{
	Super::NativeConstruct();
	
	Button->OnClicked.AddDynamic(this, &URoleButton::OnButtonClick);
}

void URoleButton::OnButtonClick()
{
	OnClicked.Broadcast(Role);
}

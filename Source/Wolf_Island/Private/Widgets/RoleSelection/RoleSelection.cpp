// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/RoleSelection/RoleSelection.h"

#include "Character/MainPlayerController.h"
#include "Components/Button.h"
#include "Components/WrapBox.h"
#include "Games/MainGameState.h"
#include "Games/MainPlayerState.h"
#include "Widgets/BaseButton.h"
#include "Widgets/RoleSelection/RoleButton.h"

void URoleSelection::NativeConstruct()
{
	Super::NativeConstruct();
	
	MainGameState = Cast<AMainGameState>(GetWorld()->GetGameState());
	PlayerController = Cast<AMainPlayerController>(GetOwningPlayer());
	
	for (UWidget* Child : RoleList->GetAllChildren())
	{
		URoleButton* Button = Cast<URoleButton>(Child);
		Button->OnClicked.AddDynamic(this, &URoleSelection::SetInfoSection);
	}
	
	ConfirmButton->OnClicked.AddDynamic(this, &URoleSelection::ConfirmSelection);
	ConfirmButton->Button->SetIsEnabled(false);
	
	CheckOccupied();
}

void URoleSelection::CheckOccupied()
{
	MainGameState = Cast<AMainGameState>(GetWorld()->GetGameState());
	
	if (!MainGameState)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainGameState is null"));
		return;
	}
	
	for (UWidget* Child : RoleList->GetAllChildren())
	{
		URoleButton* Button = Cast<URoleButton>(Child);
		
		for (APlayerState* PS : MainGameState->PlayerArray)
		{
			AMainPlayerState* Player = Cast<AMainPlayerState>(PS);
			if (Player->GetPlayerRole() == Button->Role)
			{
				Button->SetOccupied(true);
			} else
			{
				Button->SetOccupied(false);
			}
		}
	}
}

void URoleSelection::SetInfoSection_Implementation(ECharacterRole Role)
{
	SelectedRole = Role;
	ConfirmButton->Button->SetIsEnabled(true);
}

void URoleSelection::ConfirmSelection()
{
	if (!PlayerController) return;
	
	UE_LOG(LogTemp, Warning, TEXT("Confirmed Role : %d"), SelectedRole);
	PlayerController->Server_ConfirmRole(SelectedRole);
	RemoveFromParent();
}

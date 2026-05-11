// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/RoleSelection/RoleSelection.h"

#include "Character/MainPlayerController.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Games/MainGameState.h"
#include "Widgets/BaseButton.h"
#include "Widgets/RoleSelection/RoleButton.h"

void URoleSelection::NativeConstruct()
{
	Super::NativeConstruct();
	
	MainGameState = Cast<AMainGameState>(GetWorld()->GetGameState());
	PlayerController = Cast<AMainPlayerController>(GetOwningPlayer());
	
	if (MainGameState)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainGameState is valid"));
		MainGameState->OnSelectedRolesChanged.AddDynamic(this, &URoleSelection::CheckOccupied);
		UE_LOG(LogTemp, Warning, TEXT("OnSelectedRolesChanged Binded"));
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainGameState is invalid"));
	}
	
	for (UWidget* Child : RoleList->GetAllChildren())
	{
		URoleButton* Button = Cast<URoleButton>(Child);
		Button->OnClicked.AddDynamic(this, &URoleSelection::SetInfoSection);
	}
	
	ConfirmButton->OnClicked.AddDynamic(this, &URoleSelection::ConfirmSelection);
	ConfirmButton->Button->SetIsEnabled(false);
	
	CheckOccupied();
	
	SetRandomRole();
}

void URoleSelection::CheckOccupied()
{
	//MainGameState = Cast<AMainGameState>(GetWorld()->GetGameState());
	
	if (!MainGameState)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainGameState is null"));
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[ROLE SELECTION UI] Occupied Check"))
	for (UWidget* Child : RoleList->GetAllChildren())
	{
		URoleButton* Button = Cast<URoleButton>(Child);
		
		for (const ECharacterRole& OccupiedRole : MainGameState->SelectedRoles)
		{
			if (OccupiedRole == Button->Role)
			{
				Button->Button->SetIsEnabled(false);
				break;
			} else
			{
				Button->Button->SetIsEnabled(true);
				break;
			}
		}
	}
}

void URoleSelection::SetInfoSection_Implementation(ECharacterRole Role)
{
	SelectedRole = Role;
	ConfirmButton->Button->SetIsEnabled(true);
	
	for (UWidget* Child : RoleList->GetAllChildren())
	{
		if (URoleButton* Button = Cast<URoleButton>(Child))
		{
			if (Button->Role == SelectedRole)
			{
				RoleName->SetText(Button->RoleData.RoleName);
				RoleDesc->SetText(Button->RoleData.RoleDescription);
				RoleThumbnail->SetBrushFromTexture(Button->RoleData.RoleThumbnail);
			}
		}
	}
}

void URoleSelection::ConfirmSelection()
{
	if (!PlayerController) return;
	
	UE_LOG(LogTemp, Warning, TEXT("Confirmed Role : %d"), SelectedRole);
	PlayerController->Server_ConfirmRole(SelectedRole);
}

void URoleSelection::PlayDenyAlarm()
{
	if (DenyAlarm)
	{
		PlayAnimation(DenyAlarm);
	}
}

void URoleSelection::SetRandomRole()
{
	MainGameState = Cast<AMainGameState>(GetWorld()->GetGameState());
	
	for (UWidget* Child : RoleList->GetAllChildren())
	{
		if (URoleButton* Button = Cast<URoleButton>(Child))
		{
			if (MainGameState->CheckAvailableRole(Button->Role))
			{
				Button->OnClicked.Broadcast(Button->Role);
				return;
			}
		}
	}
}

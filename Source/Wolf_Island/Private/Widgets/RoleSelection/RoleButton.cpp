// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/RoleSelection/RoleButton.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void URoleButton::NativeConstruct()
{
	Super::NativeConstruct();
	
	Button->OnClicked.AddDynamic(this, &URoleButton::OnButtonClick);
	
	//데이터테이블에서 역할 정보 가져와서 이름과 설명 세팅
	if (RoleDataTable)
	{
		FString EnumString = StaticEnum<ECharacterRole>()->GetNameStringByValue((uint8)Role);
		FName RowName(*EnumString);
		const FRoleData* FoundRoleData = RoleDataTable->FindRow<FRoleData>(RowName, "RoleData");
		RoleData = *FoundRoleData;
		
		if (FoundRoleData)
		{
			RoleName->SetText(RoleData.RoleName);
			RoleDesc->SetText(RoleData.RoleDescription);
			
			if (RoleData.RoleIcon)
			{
				RoleIcon->SetBrushFromTexture(RoleData.RoleIcon);
			}
		}
	}
}

void URoleButton::OnButtonClick()
{
	OnClicked.Broadcast(Role);
}

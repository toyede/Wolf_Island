// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MainPlayerController.h"

#include "Games/MainHUD.h"

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	HUD = Cast<AMainHUD>(GetHUD());
}

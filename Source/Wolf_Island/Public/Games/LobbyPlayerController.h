// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

enum class ECharacterRole : uint8;
/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	bool IsReady = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UUserWidget* LobbyWidget;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> LobbyWidgetClass;
	
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	void OnRep_PlayerState() override;
	
	UFUNCTION(BlueprintCallable)
	void Request_ToggleReady();
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_ToggleReady();
	UFUNCTION(BlueprintCallable)
	void ToggleReady();
	
	UFUNCTION(BlueprintCallable)
	void Request_ChangeRole(ECharacterRole NewRole);
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_ChangeRole(ECharacterRole NewRole);
	UFUNCTION(BlueprintCallable)
	void ChangeRole(ECharacterRole NewRole);
};

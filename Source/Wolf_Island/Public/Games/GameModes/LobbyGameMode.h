// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MultiGameMode.h"
#include "LobbyGameMode.generated.h"

class APlayerSlot;
/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool SlotsDone = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<APlayerSlot*> PlayerSlots;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<APlayerController*> PlayerControllers;
	
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void OnPostLogin(AController* NewPlayer) override;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	virtual void Logout(AController* Exiting) override;
	
	UFUNCTION(BlueprintCallable)
	void SetPlayerSlots();
	
	UFUNCTION(BlueprintCallable)
	void AllocateSlot(APlayerController* NewController);
	
	UFUNCTION(BlueprintCallable)
	void RemoveSlot(APlayerController* RemovedController);
	
	UFUNCTION(BlueprintCallable)
	void RefreshSlots();
	
	UFUNCTION(BlueprintCallable)
	void RefreshSlot(APlayerController* TargetController);
	
	UFUNCTION(BlueprintCallable)
	bool CheckAllPlayerReady();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void RunGameTravel();
	
	UFUNCTION(BlueprintCallable)
	bool CheckRoleSelection();
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MainGameState.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUnlockedRecordsChanged);

UENUM(BlueprintType)
enum class EMessageType : uint8
{
	GENERAL,
	NOTICE,
	ALARM,
	ALERT
};

USTRUCT(BlueprintType)
struct FChattingData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	EMessageType MessageType = EMessageType::GENERAL;
	
	UPROPERTY(BlueprintReadWrite)
	FString Name;
	
	UPROPERTY(BlueprintReadWrite)
	FString Message;
	
	FChattingData() = default;

	FChattingData(const FString& InName, const FString& InMessage, EMessageType InMessageType = EMessageType::GENERAL)
		: MessageType(InMessageType), Name(InName), Message(InMessage){}
	
	bool IsEmpty()
	{
		if (!Name.IsEmpty()) return false;
		if (!Message.IsEmpty()) return false;
		
		return true;
	}
};

UCLASS()
class WOLF_ISLAND_API AMainGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(Replicated, BlueprintReadWrite)
	TArray<FChattingData> ChattingData;
	
	UFUNCTION(BlueprintCallable)
	void AddChattingMessage(FChattingData NewChattingData);
	
	UFUNCTION(BlueprintCallable)
	FChattingData GetLastChattingData();
	
	virtual void BeginPlay() override;
	
	//멀티플레이 코드
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(NetMulticast, Reliable)
	void Multi_AddChat(FChattingData NewChattingData);

	// 해금 시스템 (알 수 없는 기록)
	UPROPERTY(ReplicatedUsing = OnRep_UnlockedRecordIDs, BlueprintReadOnly, Category = "Records")
	TArray<FString> UnlockedRecordIDs;
	
	UPROPERTY(BlueprintAssignable, Category = "Records")
	FOnUnlockedRecordsChanged OnUnlockedRecordsChanged;

	UFUNCTION(BlueprintCallable, Category = "Records")
	void UnlockRecord(const FString& RecordID);

	UFUNCTION()
	void OnRep_UnlockedRecordIDs();
};

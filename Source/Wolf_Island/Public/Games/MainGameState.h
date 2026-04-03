// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MainPlayer.h"
#include "GameFramework/GameState.h"
#include "MainGameState.generated.h"

/**
 * 
 */

struct FPlayerSaveData;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUnlockedRecordsChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSelectedRolesChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSharedRecipesChanged);

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
	
	UPROPERTY(BlueprintReadWrite)
	int64 UnixTime;
	
	FChattingData()
	{
		UnixTime = FDateTime::UtcNow().ToUnixTimestamp();
	};

	FChattingData(const FString& InName, const FString& InMessage, EMessageType InMessageType = EMessageType::GENERAL)
		: MessageType(InMessageType), Name(InName), Message(InMessage)
	{
		UnixTime = FDateTime::UtcNow().ToUnixTimestamp();
	}
	
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
	//현재 게임이 멀티인지 싱글인지
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool IsMulti = false;
	
	//선택한 역할들이 바뀌면
	FOnSelectedRolesChanged OnSelectedRolesChanged;
	
	//채팅창 채팅 리스트
	UPROPERTY(Replicated, BlueprintReadWrite)
	TArray<FChattingData> ChattingData;
	
	//현재 입장한 플레이어들이 선택한 역할들
	UPROPERTY(ReplicatedUsing=OnRep_SelectedRoles, BlueprintReadWrite)
	TArray<ECharacterRole> SelectedRoles;
	
	//선택한 역할 세팅하는 함수
	UFUNCTION(BlueprintCallable)
	void RefreshSelectedRoles();
	
	//역할 중복 확인 함수
	UFUNCTION(BlueprintCallable)
	bool CheckAvailableRole(ECharacterRole NewRole);
	
	//사용 가능한 역할 리스트 반환 함수
	UFUNCTION(BlueprintCallable)
	TArray<ECharacterRole> GetAvailableRoles();
	
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
	
	UFUNCTION()
	void OnRep_SelectedRoles();

	// 공유 레시피
	UPROPERTY(ReplicatedUsing = OnRep_SharedRecipes, BlueprintReadOnly, Category = "Crafting")
	TArray<FName> SharedRecipes;

	// UI 업데이트용 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSharedRecipesChanged OnSharedRecipesChanged;

	// 공유 레시피 해금
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void UnlockSharedRecipe(const FName& RecipeID);

	// 클라이언트 동기화 함수
	UFUNCTION()
	void OnRep_SharedRecipes();
};

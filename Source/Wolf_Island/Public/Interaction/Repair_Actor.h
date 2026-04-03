#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableActor.h"
#include "Data/ItemDataStruct.h"
#include "Engine/DataTable.h"
#include "Repair_Actor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRepairStatusChanged);

UCLASS()
class WOLF_ISLAND_API ARepair_Actor : public AInteractableActor
{
	GENERATED_BODY()

	virtual void BeginPlay() override;
	
public:

	ARepair_Actor();

	virtual void Interact_Implementation(AActor* Interactor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class URepairUI> RepairUIClass;

	UFUNCTION(Client, Reliable)
	void Client_OpenRepairUI(class APlayerController* PC);

	UPROPERTY(ReplicatedUsing = OnRep_CompletedRecipes, BlueprintReadOnly, Category = "Repair", SaveGame)
	TArray<FName> CompletedRecipeNames;

	UFUNCTION()
	void OnRep_CompletedRecipes();

	UPROPERTY(BlueprintAssignable, Category = "Repair")
	FOnRepairStatusChanged OnRepairStatusChanged;

	UPROPERTY(ReplicatedUsing = OnRep_RepairStatus, EditAnywhere, BlueprintReadWrite, Category = "Check", SaveGame)
	bool bIsBody = false;
	UPROPERTY(ReplicatedUsing = OnRep_RepairStatus, EditAnywhere, BlueprintReadWrite, Category = "Check", SaveGame)
	bool bIsEngine = false;
	UPROPERTY(ReplicatedUsing = OnRep_RepairStatus, EditAnywhere, BlueprintReadWrite, Category = "Check", SaveGame)
	bool bIsSteering = false;
	UPROPERTY(ReplicatedUsing = OnRep_RepairStatus, EditAnywhere, BlueprintReadWrite, Category = "Check", SaveGame)
	bool bIsRadar = false;
	UPROPERTY(ReplicatedUsing = OnRep_RepairStatus, EditAnywhere, BlueprintReadWrite, Category = "Check", SaveGame)
	bool bIsAnchor = false;
	
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool CheckBodyComplete();
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool CheckEngineComplete();
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool CheckSteeringComplete();
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool CheckRadarComplete();
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool CheckAnchorComplete();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION(BlueprintCallable, Category = "Data")
	void CompleteRepair();
	
	UPROPERTY(EditDefaultsOnly)
	UDataTable* RepairRecipesTable;

	UPROPERTY(SaveGame)
	TMap<FName, bool> RepairStatusMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Repair|State", SaveGame)
	TMap<FName, FName> RecipeIDMap;
	
	// Runtime-only cache (not a UPROPERTY because TMap<..., TArray<...>> is not supported by UPROPERTY)
	TMap<FName, TArray<FName>> SortToRecipeRows;

	UFUNCTION(BlueprintCallable, Category = "Repair")
	void MarkRecipeAsComplete(FName RecipeName);

	UFUNCTION(BlueprintCallable, Category = "Repair")
	bool IsRecipeComplete(FName TargetRecipeName);
	
	UFUNCTION(BlueprintCallable, Category = "Repair")
	bool IsSortComplete(FName SortKey);

	UFUNCTION(BlueprintCallable, Category = "Repair")
	void RestoreStateFromGameInstance();

	bool bHasLevelLoadStarted = false;

	UFUNCTION()
	void OnRep_RepairStatus();

	UFUNCTION(BlueprintImplementableEvent, Category = "Repair")
	void UpdateShipVisuals();
	
	//저장 관련 코드
	virtual void SaveData_Implementation(FActorSaveData& OutData) override;
	virtual void LoadData_Implementation(const FActorSaveData& InData) override;

	
	// 싱글 전용 늑대인간으로 변했을 때 수리된 것들 중 랜덤으로 파괴되는 기능
	// 현재 파괴 가능한 수리 항목 목록 반환
	UFUNCTION(BlueprintCallable, Category = "Repair")
	TArray<FName> GetBreakableRecipes() const;

	// 지정 항목 1개를 완료 상태에서 미완료 상태로 되돌림
	UFUNCTION(BlueprintCallable, Category = "Repair")
	bool BreakCompletedRepair(FName RecipeName);

	// 후보 목록 중 랜덤 1개 선택해서 BreakCompletedRepair() 호출
	UFUNCTION(BlueprintCallable, Category = "Repair")
	bool BreakRandomCompletedRepair();

	// 부위별 bool과 비주얼을 다시 맞춤
	UFUNCTION(BlueprintCallable, Category = "Repair")
	void RefreshRepairProgressState();

	// ******게임 클리어 관련******
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Escape")
	class UBoxComponent* EscapeReadyVolume;

	// 탈출 시도
	UFUNCTION(BlueprintCallable, Category = "Escape")
	void TryEscape(AActor* Interactor);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// 대기 구역 오버랩 이벤트
	UFUNCTION()
	void OnEscapeVolumeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEscapeVolumeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 페이드 아웃 및 시네마틱 연출 
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayEscapeCinematic();

	// 실제 맵 이동 로직
	void ExecuteMapTransition();

private:
	bool AreAllPlayersInVolume() const;

	bool IsAnyPlayerInfected() const;
	
	// 현재 대기 구역에 있는 플레이어 목록
	UPROPERTY()
	TSet<class AMainPlayer*> PlayersInVolume;

	// 시네마틱 재생을 위한 타이머 핸들
	FTimerHandle CinematicTimerHandle;

	bool bIsEscaping = false;
};

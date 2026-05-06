#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PrayerAltar.generated.h"

class UBoxComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class AMainPlayer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPrayerSuccess);

UCLASS()
class WOLF_ISLAND_API APrayerAltar : public AActor
{
	GENERATED_BODY()

public:
	APrayerAltar();

protected:
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Prayer")
	FOnPrayerSuccess OnPrayerSuccess;

	// 현재 제단에 있는 플레이어들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prayer")
	TArray<AMainPlayer*> PlayersOnAltar;

	// 요구되는 기도 감정표현의 태그나 ID (현재는 임시)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prayer")
	FName RequiredEmoteID = TEXT("Prayer");

protected:
	UPROPERTY(VisibleAnywhere, Category = "Prayer")
	UBoxComponent* AltarZone;

	UPROPERTY(VisibleAnywhere, Category = "Prayer|Visual")
	UStaticMeshComponent* AltarMesh;

	UPROPERTY(EditAnywhere, Category = "Prayer|Effects")
	UNiagaraSystem* SuccessEffect;

private:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	// 플레이어가 감정표현을 시작했을 때 호출할 함수 (나중에 MainPlayer에서 호출)
	void NotifyEmoteStarted(AMainPlayer* Player, FName EmoteID);
};

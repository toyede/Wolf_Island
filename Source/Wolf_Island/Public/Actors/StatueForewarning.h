#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StatueForewarning.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;

DECLARE_MULTICAST_DELEGATE(FOnForewarningComplete);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnForewarningResolved, bool /*bAreaClear*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTelegraphStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTelegraphResolved, bool, bAreaClear, int32, EliminatedPlayers);

UCLASS()
class WOLF_ISLAND_API AStatueForewarning : public AActor
{
	GENERATED_BODY()

public:
	AStatueForewarning();

protected:
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Legacy completion signal
	FOnForewarningComplete OnForewarningComplete;
	// New completion signal with area state
	FOnForewarningResolved OnForewarningResolved;

	UPROPERTY(BlueprintAssignable, Category = "Forewarning")
	FOnTelegraphStarted OnTelegraphStarted;

	UPROPERTY(BlueprintAssignable, Category = "Forewarning")
	FOnTelegraphResolved OnTelegraphResolved;

	UFUNCTION(BlueprintImplementableEvent, Category = "Forewarning")
	void BP_OnTelegraphStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Forewarning")
	void BP_OnTelegraphResolved(bool bAreaClear, int32 EliminatedPlayers);

protected:
	UPROPERTY(EditAnywhere, Category = "Forewarning")
	float ForewarningDuration = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Forewarning")
	float LethalDamage = 99999.0f;

	UPROPERTY(VisibleAnywhere, Category = "Forewarning")
	USphereComponent* KillZone;

	UPROPERTY(EditAnywhere, Category = "Forewarning|Effects")
	UNiagaraSystem* ForewarningEffect;

	UPROPERTY(VisibleAnywhere, Category = "Forewarning|Effects")
	UNiagaraComponent* ForewarningEffectComponent;

	//KSH-경고 사운드 (플레이어가 위험 지역을 인지할 수 있도록)
	UPROPERTY(EditAnywhere, Category = "Forewarning|Effects")
	USoundBase* ForewarningSound;

	//KSH-이펙트 스케일을 KillZone 반경에 맞출지 여부
	UPROPERTY(EditAnywhere, Category = "Forewarning|Effects")
	bool bMatchEffectScaleToKillZone = true;

	//KSH-스케일 1일 때 이펙트가 표현하는 반경(cm). 나이아가라 에셋에 맞춰 조정
	UPROPERTY(EditAnywhere, Category = "Forewarning|Effects", meta = (ClampMin = "1.0"))
	float EffectBaseRadius = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Forewarning|Debug")
	bool bDebugDrawWarning = true;

	UPROPERTY(EditAnywhere, Category = "Forewarning|Debug")
	FColor DebugSphereColor = FColor::Yellow;

private:
	void OnForewarningEnd();
	int32 KillOverlappingPlayers();
	bool HasOverlappingPlayers() const;

	FTimerHandle ForewarningTimerHandle;
};

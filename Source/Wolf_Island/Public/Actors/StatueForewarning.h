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

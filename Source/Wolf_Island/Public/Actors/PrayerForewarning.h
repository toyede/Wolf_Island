#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PrayerForewarning.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPrayerForewarningComplete);

UCLASS()
class WOLF_ISLAND_API APrayerForewarning : public AActor
{
	GENERATED_BODY()

public:
	APrayerForewarning();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Prayer")
	FOnPrayerForewarningComplete OnForewarningComplete;

protected:
	UPROPERTY(EditAnywhere, Category = "Prayer")
	float Duration = 2.0f;

	UPROPERTY(VisibleAnywhere, Category = "Prayer")
	USphereComponent* AreaIndicator;

	UPROPERTY(EditAnywhere, Category = "Prayer|Effects")
	UNiagaraSystem* ForewarningEffect;

	UPROPERTY(VisibleAnywhere, Category = "Prayer|Effects")
	UNiagaraComponent* ForewarningEffectComponent;

private:
	void OnTimerEnd();
	FTimerHandle TimerHandle;
};

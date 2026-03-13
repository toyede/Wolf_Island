// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SavableActor.h"
#include "GameFramework/Actor.h"
#include "BarricadeTrap.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class USoundBase;

UCLASS()
class WOLF_ISLAND_API ABarricadeTrap : public ASavableActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABarricadeTrap();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* DamageBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap Settings")
	float DamageAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap Settings")
	float DamageInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap Settings")
	int32 MaxAttackCount;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 사운드 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap Settings|Effects")
	USoundBase* DestroySound;

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayDestroyEffects();
	
private:
	void DealPeriodicDamage();
    
	void PerformAttack(AActor* TargetActor);

	FTimerHandle DamageTimerHandle;
    
	UPROPERTY()
	TSet<AActor*> OverlappingActors;

	int32 CurrentAttackCount;

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AI/AIControllers/EnemyAIBossController.h"
#include "EnemyAIBoss.generated.h"

class UAnimMontage;
class UAttackCollisionComponent;
class UStatusComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossAttackEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossRushEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossGroggyEnd);

UCLASS()
class WOLF_ISLAND_API AEnemyAIBoss : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyAIBoss();

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnBossAttackEnd OnBossAttackEnd;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnBossRushEnd OnBossRushEnd;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnBossGroggyEnd OnBossGroggyEnd;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Comp")
	TObjectPtr<UStatusComponent> StatusComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Comp")
	TObjectPtr<UAttackCollisionComponent> AttackCollisionComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TArray<UAnimMontage*> AttackMontages;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* RushMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* GroggyMontage;

	UFUNCTION()
	void ExecuteAttack(int32 AttackIndex);

	UFUNCTION()
	void ExecuteRush();

	UFUNCTION()
	void ExecuteGroggy();

	UFUNCTION()
	void EndGroggy();

protected:
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnRushMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnGroggyMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected:
	UPROPERTY(EditAnywhere, Category = "Groggy")
	float GroggyDuration = 1.f;
private:
	FTimerHandle GroggyTimerHandle;
};

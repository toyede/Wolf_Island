// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AI/AIControllers/EnemyAIBossController.h"
#include "AI/Interfaces/AttackMeshProvider.h"
#include "EnemyAIBoss.generated.h"

class UAnimMontage;
class UAttackCollisionComponent;
class UStatusComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossAttackEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossRushEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossGroggyEnd);

UCLASS()
class WOLF_ISLAND_API AEnemyAIBoss : public ACharacter, public IAttackMeshProvider
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

	// 대미지

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TArray<float> AttackDamages;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float RushDamage = 20.f;

	UPROPERTY()
	float CurrentDamage = 0.f;

	void SetCurrentDamage(float Damage) { CurrentDamage = Damage; }

	// 공격 범위 소켓

	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	TArray<FName> AttackStartSockets;

	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	TArray<FName> AttackEndSockets;

	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	FName RushStartSocket;

	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	FName RushEndSocket;

	// 콜리전 범위

	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	TArray<float> AttackRadiuses;

	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	float RushRadius = 50.f;

	// 몽타주

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

	UFUNCTION()
	void OnAttackHit(const FHitResult& HitResult);

	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnRushMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnGroggyMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected:
	UPROPERTY(EditAnywhere, Category = "Groggy")
	float GroggyDuration = 1.f;

private:
	FTimerHandle GroggyTimerHandle;

public:
	virtual USkeletalMeshComponent* GetAttackMesh() const override
	{
		return GetMesh();
	}

	virtual UAttackCollisionComponent* GetAttackCollisionComponent() const override
	{
		return AttackCollisionComponent;
	}

protected:
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	UPROPERTY()
	bool bIsRushing = false;
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "AI/Interfaces/AttackMeshProvider.h"
#include "AI/Interfaces/EnemyCommonInterface.h"
#include "WerewolfInfected.generated.h"

class UAttackCollisionComponent;
class UCameraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInfectedOnHitResponse); // 맞을 때 피격 모션 바인딩용
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInfectedOnAttackEnd); // 기본 공격 끝났음을 알리는 용도

UCLASS()
class WOLF_ISLAND_API AWerewolfInfected : public ACharacter, public IEnemyCommonInterface, public IAttackMeshProvider
{
    GENERATED_BODY()

public:
    AWerewolfInfected();

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FInfectedOnHitResponse OnHitResponse;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FInfectedOnAttackEnd OnAttackEnd;

protected:
    virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
public:
    // === 컴포넌트 ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Werewolf|Camera")
    TObjectPtr<UCameraComponent> FirstPersonCamera;

    // === 체력 ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Werewolf|Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, BlueprintReadOnly, Category = "Werewolf|Stats")
    float CurrentHealth = 100.0f;

    UFUNCTION()
    void OnRep_CurrentHealth();

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    // === 기절 (HP 10% 이하) ===
    UPROPERTY(ReplicatedUsing = OnRep_Incapacitated, BlueprintReadOnly, Category = "Werewolf|State")
    bool bIsIncapacitated = false;

    UFUNCTION()
    void OnRep_Incapacitated();

    UPROPERTY(EditAnywhere, Category = "Werewolf|Stats")
    float IncapacitateThreshold = 0.1f;  // 10%

    // === 공격 ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Werewolf|Combat")
    TObjectPtr<UAttackCollisionComponent> AttackCollisionComp;

    UFUNCTION(Server, Reliable)
    void Server_RequestAttack();

    UPROPERTY(EditAnywhere, Category = "Werewolf|Combat")
    TObjectPtr<UAnimMontage> AttackMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Werewolf|Combat")
    TSubclassOf<UDamageType> InfectedAttackDamageType;

    UFUNCTION()
    void OnAttackHit(const FHitResult& HitResult);

	UPROPERTY(EditDefaultsOnly, Category = "Werewolf|Combat")
	float AttackDamage = 10.0f;

    // === 관전 전환 지원 ===
    UPROPERTY(BlueprintReadOnly, Category = "Werewolf|State")
    bool bIsPlayerControlled = true;

    // === 리플리케이션 ===
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual void Die_Implementation() override;

    virtual void NormalAttack_Implementation() override;

private:
    void HandleIncapacitated();

    // 입력 바인딩용
    void OnAttackInput();

private:

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayAttack();

    UPROPERTY(EditAnywhere, Category = "Werewolf|AI")
    float AITickInterval = 0.2f;

    UPROPERTY(EditAnywhere, Category = "Werewolf|AI")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Werewolf|AI")
    float DetectionRange = 3000.0f;

    bool bIsAttacking = false;

    FTimerHandle AttackResetHandle;

	// / === 관전 전환 관련 ===
public:
    UPROPERTY(EditDefaultsOnly, Category = "Werewolf|Input")
    TObjectPtr<UInputMappingContext> SpectateIMC;

    UPROPERTY(EditDefaultsOnly, Category = "Werewolf|Input")
    TObjectPtr<UInputAction> SwitchSpectateAction;

    void SwitchSpectateTarget();

	// === IAttackMeshProvider 인터페이스 구현 ===
public:
    virtual USkeletalMeshComponent* GetAttackMesh() const override;
	virtual UAttackCollisionComponent* GetAttackCollisionComponent() const override;

	// === 피격 처리 ===
public:
    // 피격 시 재생할 애니메이션 몽타주
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* HitMontage;

protected:
    // 피격 처리 (서버 전용)
    void HitResponse();

    // 피격 연출 (모든 클라이언트)
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_HitResponse();

    // 피격 몽타주 종료 콜백
    void OnHitMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    // 기절 시 재생할 애니메이션 몽타주
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* IncapacitatedMontage;
};
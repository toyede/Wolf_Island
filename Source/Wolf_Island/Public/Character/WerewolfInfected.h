#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "AI/Interfaces/AttackMeshProvider.h"
#include "AI/Interfaces/EnemyCommonInterface.h"
#include "NiagaraSystem.h"
#include "WerewolfInfected.generated.h"

class UAttackCollisionComponent;
class UCameraComponent;
class UParticleSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInfectedOnHitResponse); // ���� �� �ǰ� ��� ���ε���
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInfectedOnAttackEnd); // �⺻ ���� �������� �˸��� �뵵

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
    // === ������Ʈ ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Werewolf|Camera")
    TObjectPtr<UCameraComponent> FirstPersonCamera;

    // === ü�� ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Werewolf|Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, BlueprintReadOnly, Category = "Werewolf|Stats")
    float CurrentHealth = 100.0f;

    UFUNCTION()
    void OnRep_CurrentHealth();

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    // === ���� (HP 10% ����) ===
    UPROPERTY(ReplicatedUsing = OnRep_Incapacitated, BlueprintReadOnly, Category = "Werewolf|State")
    bool bIsIncapacitated = false;

    UFUNCTION()
    void OnRep_Incapacitated();

    UPROPERTY(EditAnywhere, Category = "Werewolf|Stats")
    float IncapacitateThreshold = 0.1f;  // 10%

    // === ���� ===
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

    // === ���� ��ȯ ���� ===
    UPROPERTY(BlueprintReadOnly, Category = "Werewolf|State")
    bool bIsPlayerControlled = true;

    // === ���ø����̼� ===
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual void Die_Implementation() override;

    virtual void NormalAttack_Implementation() override;

private:
    void HandleIncapacitated();

    // �Է� ���ε���
    void OnAttackInput();

	// === 피격 이펙트/사운드 (HitResponse 확률 없이 무조건 재생) ===
	UPROPERTY(EditDefaultsOnly, Category = "Werewolf|Hit")
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditDefaultsOnly, Category = "Werewolf|Hit")
	TObjectPtr<UNiagaraSystem> HitEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Werewolf|Hit")
	TObjectPtr<UParticleSystem> HitEffectCascade;

	UPROPERTY(EditDefaultsOnly, Category = "Werewolf|Hit")
	FName HitEffectSocketName = NAME_None;

private:

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayAttack();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayHitEffect(FVector HitLocation, FVector HitNormal);

    UPROPERTY(EditAnywhere, Category = "Werewolf|AI")
    float AITickInterval = 0.2f;

    UPROPERTY(EditAnywhere, Category = "Werewolf|AI")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Werewolf|AI")
    float DetectionRange = 3000.0f;

    bool bIsAttacking = false;

    FTimerHandle AttackResetHandle;

	// / === ���� ��ȯ ���� ===
public:
    UPROPERTY(EditDefaultsOnly, Category = "Werewolf|Input")
    TObjectPtr<UInputMappingContext> SpectateIMC;

    UPROPERTY(EditDefaultsOnly, Category = "Werewolf|Input")
    TObjectPtr<UInputAction> SwitchSpectateAction;

    void SwitchSpectateTarget();

	// === IAttackMeshProvider �������̽� ���� ===
public:
    virtual USkeletalMeshComponent* GetAttackMesh() const override;
	virtual UAttackCollisionComponent* GetAttackCollisionComponent() const override;

	// === �ǰ� ó�� ===
public:
    // �ǰ� �� ����� �ִϸ��̼� ��Ÿ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* HitMontage;

protected:
    // �ǰ� ó�� (���� ����)
    void HitResponse();

    // �ǰ� ���� (��� Ŭ���̾�Ʈ)
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_HitResponse();

    // �ǰ� ��Ÿ�� ���� �ݹ�
    void OnHitMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    // ���� �� ����� �ִϸ��̼� ��Ÿ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* IncapacitatedMontage;
};
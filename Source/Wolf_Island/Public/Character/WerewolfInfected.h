#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "AI/Interfaces/AttackMeshProvider.h"
#include "WerewolfInfected.generated.h"

class UAttackCollisionComponent;
class UCameraComponent;

UCLASS()
class WOLF_ISLAND_API AWerewolfInfected : public ACharacter, public IAttackMeshProvider
{
    GENERATED_BODY()

public:
    AWerewolfInfected();

protected:
    virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

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

    // === 관전 전환 지원 ===
    UPROPERTY(BlueprintReadOnly, Category = "Werewolf|State")
    bool bIsPlayerControlled = true;

    // === 리플리케이션 ===
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// === AI 관련 ===
    void StartAI();

private:
    void HandleIncapacitated();

    // 입력 바인딩용
    void OnAttackInput();

private:
    FTimerHandle AITickHandle;

    UPROPERTY()
    TWeakObjectPtr<ACharacter> CurrentTarget;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayAttack();

    void AITick();
    void FindClosestPlayer();
    void TryAttack();

    UPROPERTY(EditAnywhere, Category = "Werewolf|AI")
    float AITickInterval = 0.2f;

    UPROPERTY(EditAnywhere, Category = "Werewolf|AI")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Werewolf|AI")
    float DetectionRange = 3000.0f;

    bool bIsAttacking = false;
    bool bShouldMove = false;

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
};
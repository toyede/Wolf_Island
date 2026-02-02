// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifies/AnimNotify_ThrustImpact.h"
#include "AI/Enemy_Character/EnemyAIBoss.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/DamageEvents.h"

void UAnimNotify_ThrustImpact::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	AEnemyAIBoss* Boss = Cast<AEnemyAIBoss>(MeshComp->GetOwner());
	if (!Boss) return;

	// 서버에서만 실행
	if (!Boss->HasAuthority()) return;

	FVector BossLocation = Boss->GetActorLocation();

	// 주변 원형 범위 내 액터 탐색
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Boss);

	TArray<AActor*> FoundActors;

	UKismetSystemLibrary::SphereOverlapActors(
		Boss->GetWorld(),
		BossLocation,
		ThrustRange,
		ObjectTypes,
		ACharacter::StaticClass(),
		IgnoreActors,
		FoundActors
	);

	for (AActor* Actor : FoundActors)
	{
		ACharacter* TargetChar = Cast<ACharacter>(Actor);
		if (!TargetChar) continue;

		// 밀쳐내는 방향: 보스 → 타겟
		FVector ToTarget = (TargetChar->GetActorLocation() - BossLocation).GetSafeNormal2D();
		FVector LaunchVelocity = ToTarget * ThrustForce;
		LaunchVelocity.Z = UpwardForce;

		// 데미지 적용
		if (ThrustDamage > 0.f)
		{
			FDamageEvent DamageEvent;
			TargetChar->TakeDamage(ThrustDamage, DamageEvent, Boss->GetController(), Boss);
		}

		// 넉백 적용
		TargetChar->LaunchCharacter(LaunchVelocity, true, true);
	}
}

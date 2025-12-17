// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Trap_Base.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "AI/Enemy_Character/EnemyAIBase.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ATrap_Base::ATrap_Base()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TrapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrapMesh"));
	RootComponent = TrapMesh;

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->SetupAttachment(RootComponent);
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	TrapSpan = 5.f;
}

// Called when the game starts or when spawned
void ATrap_Base::BeginPlay()
{
	Super::BeginPlay();

	TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ATrap_Base::OnOverlapBegin);
}

// Called every frame
void ATrap_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATrap_Base::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Enemy"))
	{
		AEnemyAIBase* Enemy = Cast<AEnemyAIBase>(OtherActor);
		if (Enemy)
		{
			TrappedEnemy = Enemy;

			FVector TrapCenter = GetActorLocation();
			FVector EnemyLoc = Enemy->GetActorLocation();
			Enemy->SetActorLocation(FVector(TrapCenter.X, TrapCenter.Y, EnemyLoc.Z));

			Enemy->GetCharacterMovement()->DisableMovement();

			// SetLifeSpan 대신 타이머 사용
			GetWorldTimerManager().SetTimer(ReleaseTimerHandle, this, &ATrap_Base::ReleaseTrap, TrapSpan, false);
		}
	}
}

void ATrap_Base::ReleaseTrap()
{
	if (TrappedEnemy && IsValid(TrappedEnemy))
	{
		TrappedEnemy->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	Destroy();
}



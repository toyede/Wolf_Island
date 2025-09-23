// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifies/ThrowStoneAnimNotify.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "AIController.h"


void UThrowStoneAnimNotify::Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	if (!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;
	
	
	
	FVector MuzzleLocation = MeshComp->GetSocketLocation("Hand_R"); // �� �� ���� �̸�
	FVector TargetLocation = Owner->GetActorLocation();

	UWorld* World = Owner->GetWorld();
	if (World)
	{
		World->SpawnActor<AStoneProjectile>(ProjectileClass, MuzzleLocation, Owner->GetActorRotation());
	}
}

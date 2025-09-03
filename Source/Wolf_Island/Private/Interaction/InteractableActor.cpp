// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/InteractableActor.h"

// Sets default values
AInteractableActor::AInteractableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AInteractableActor::BeginPlay()
{
	Super::BeginPlay();

	InteractableData.InteractionDuration = InteractionDuration;
}

// Called every frame
void AInteractableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AInteractableActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	//바뀐 프로퍼티 이름 가져오기
	const FName ChangedPropertyName = PropertyChangedEvent.Property ? FName(PropertyChangedEvent.Property->GetName()) : NAME_None;

	//프로퍼티의 이름이 InteractionDuration 이면
	if (ChangedPropertyName == FName("InteractionDuration"))
	{
		UE_LOG(LogTemp, Warning, TEXT("Change duration"));
		//인터페이스의 시간을 디테일 창의 시간으로 업데이트
		InteractableData.InteractionDuration = InteractionDuration;
	}
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/PreviewActor.h"

// Sets default values
APreviewActor::APreviewActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false; // 프리뷰는 틱이 필요 없습니다.

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	RootComponent = MeshComponent;

	// 겹침 감지를 위한 충돌 설정
	MeshComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	MeshComponent->SetGenerateOverlapEvents(true);

}

// Called when the game starts or when spawned
void APreviewActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APreviewActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APreviewActor::SetPreviewMesh(UStaticMesh* NewMesh)
{
	if (MeshComponent && NewMesh)
	{
		MeshComponent->SetStaticMesh(NewMesh);
        
		// 머티리얼을 다이나믹으로 교체하여 색상 변경 준비
		UMaterialInterface* BaseMat = MeshComponent->GetMaterial(0);
		if (BaseMat)
		{
			GhostMaterial = MeshComponent->CreateDynamicMaterialInstance(0, BaseMat);
		}
	}
}

void APreviewActor::UpdateGhostVisual_Implementation(bool bIsAvailable)
{
	if (GhostMaterial)
	{
		// 머티리얼 파라미터 조절 (초록: 1, 빨강: 0 등 사용자가 정의한 대로)
		GhostMaterial->SetScalarParameterValue(TEXT("IsAvailable"), bIsAvailable ? 1.0f : 0.0f);
	}
}

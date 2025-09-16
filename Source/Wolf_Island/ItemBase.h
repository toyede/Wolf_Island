// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemDataStruct.h"
#include "ItemBase.generated.h"

UCLASS()
class WOLF_ISLAND_API AItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AItemBase();

	// 데이터테이블에서 읽어온 아이템 데이터 저장용 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FItemData ItemData;

	// 아이템 데이터 테이블에서 로드하는 함수
	UFUNCTION(BlueprintCallable, Category = "ItemData")
	void LoadItemDataFromTable(UDataTable* DataTable, FName RowName);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

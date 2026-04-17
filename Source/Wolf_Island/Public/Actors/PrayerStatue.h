#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PrayerStatue.generated.h"

UCLASS()
class WOLF_ISLAND_API APrayerStatue : public AActor
{
	GENERATED_BODY()

public:
	APrayerStatue();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prayer")
	FName StatuEmoteID = TEXT("Prayer");

protected:
	UPROPERTY(VisibleAnywhere, Category = "Prayer")
	UStaticMeshComponent* StatueMesh;
};

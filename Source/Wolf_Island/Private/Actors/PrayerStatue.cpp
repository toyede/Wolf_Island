#include "Actors/PrayerStatue.h"

APrayerStatue::APrayerStatue()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	StatueMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StatueMesh"));
	RootComponent = StatueMesh;
}

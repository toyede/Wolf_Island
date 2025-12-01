#include "Games/Test_ItemGameInstance.h"

void UTest_ItemGameInstance::AddActorToLevelSave(FName LevelName, FSavedActorData NewActorData)
{
	FSavedActorList& ListWrapper = LevelDataMap.FindOrAdd(LevelName);
	ListWrapper.Actors.Add(NewActorData);
}

bool UTest_ItemGameInstance::GetSavedActorsFromLevel(FName LevelName, TArray<FSavedActorData>& OutActorList)
{
	if (FSavedActorList* FoundList = LevelDataMap.Find(LevelName))
	{
		OutActorList = FoundList->Actors;
		return true;
	}

	return false;
}

void UTest_ItemGameInstance::ClearLevelData(FName LevelName)
{
	LevelDataMap.Remove(LevelName);
}

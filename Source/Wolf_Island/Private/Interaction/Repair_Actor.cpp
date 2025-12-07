#include "Interaction/Repair_Actor.h"
#include "Kismet/GameplayStatics.h"
#include "AdvancedFriendsGameInstance.h"
#include "Data/ItemDataStruct.h"

ARepair_Actor::ARepair_Actor()
{
    bIsBody = false;
    bIsEngine = false;
    bIsSteering = false;
    bIsRadar = false;
    bIsAnchor = false;
}

void ARepair_Actor::BeginPlay()
{
    

    if (RepairRecipesTable)
    {
        TArray<FName> RowNames = RepairRecipesTable->GetRowNames();
        FString ContextString;

        for (const FName& RowName : RowNames)
        {
            if (!RepairStatusMap.Contains(RowName))
            {
                RepairStatusMap.Add(RowName, false);
            }

            FRepairRecipeData* RowData = RepairRecipesTable->FindRow<FRepairRecipeData>(RowName, ContextString);
            if (RowData)
            {
                RecipeIDMap.Add(RowData->RecipeName, RowName);
                
                UE_LOG(LogTemp, Log, TEXT("[RepairActor] 매핑됨: %s -> %s"), *RowData->RecipeName.ToString(), *RowName.ToString());
            }
        }
    }
    RestoreStateFromGameInstance();

    Super::BeginPlay();
}

bool ARepair_Actor::CheckBodyComplete()
{
    bool bIsBD1Complete = IsRecipeComplete(FName("Body"));
    bool bIsBD2Complete = IsRecipeComplete(FName("Propeller"));

    if (bIsBD1Complete && bIsBD2Complete)
    {
       bIsBody = true;
    }
    else
    {
       bIsBody = false;
    }

    return bIsBody;
}

bool ARepair_Actor::CheckEngineComplete()
{
    bool bIsEG1Complete = IsRecipeComplete(FName("Engine"));
    bool bIsEG2Complete = IsRecipeComplete(FName("Pump"));

    if (bIsEG1Complete && bIsEG2Complete)
    {
       bIsEngine = true;
    }
    else
    {
       bIsEngine = false;
    }

    return bIsEngine;
}

bool ARepair_Actor::CheckSteeringComplete()
{
    bool bIsST1Complete = IsRecipeComplete(FName("Control_Device"));
    bool bIsST2Complete = IsRecipeComplete(FName("Follow-Up_Device"));

    if (bIsST1Complete && bIsST2Complete)
    {
       bIsSteering = true;
    }
    else
    {
       bIsSteering = false;
    }

    return bIsSteering;
}

bool ARepair_Actor::CheckRadarComplete()
{
    bool bIsRD1Complete = IsRecipeComplete(FName("Rader"));
    bool bIsRD2Complete = IsRecipeComplete(FName("Transmitter"));

    if (bIsRD1Complete && bIsRD2Complete)
    {
       bIsRadar = true;
    }
    else
    {
       bIsRadar = false;
    }

    return bIsRadar;
}

bool ARepair_Actor::CheckAnchorComplete()
{
    bool bIsAC1Complete = IsRecipeComplete(FName("Anchor"));
    bool bIsAC2Complete = IsRecipeComplete(FName("Lifting_Device"));

    if (bIsAC1Complete && bIsAC2Complete)
    {
       bIsAnchor = true;
    }
    else
    {
       bIsAnchor = false;
    }

    return bIsAnchor;
}

void ARepair_Actor::CompleteRepair()
{
    CheckBodyComplete();
    CheckEngineComplete();
    CheckSteeringComplete();
    CheckRadarComplete();
    CheckAnchorComplete();
    
    FString CurrentMapName = UGameplayStatics::GetCurrentLevelName(this);
    
    //if (bIsBody && bIsEngine && bIsSteering && bIsRadar && bIsAnchor)
    if (bIsBody && bIsEngine)
    {
        if (CurrentMapName == "StartMap")
        {
            return;
        }
        else
        {
            UGameplayStatics::OpenLevel(this, FName("StartMap"));
        }
        
    }
}

void ARepair_Actor::MarkRecipeAsComplete(FName RowName)
{
    if (RepairStatusMap.Contains(RowName))
    {
        RepairStatusMap[RowName] = true;
        
        UE_LOG(LogTemp, Log, TEXT("[RepairActor] 수리 완료: %s"), *RowName.ToString());

        CompleteRepair();

        UAdvancedFriendsGameInstance* GI = Cast<UAdvancedFriendsGameInstance>(GetGameInstance());
        if (GI)
        {
            GI->SaveRepairStatus(RepairStatusMap);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[RepairActor] MarkRecipe 실패: %s"), *RowName.ToString());
    }
}

bool ARepair_Actor::IsRecipeComplete(FName TargetName)
{
    if (bool* bComplete = RepairStatusMap.Find(TargetName))
    {
        return *bComplete;
    }

    if (FName* RealRowName = RecipeIDMap.Find(TargetName))
    {
        if (bool* bComplete = RepairStatusMap.Find(*RealRowName))
        {
            return *bComplete;
        }
    }

    return false;
}

void ARepair_Actor::RestoreStateFromGameInstance()
{
    UAdvancedFriendsGameInstance* GI = Cast<UAdvancedFriendsGameInstance>(GetGameInstance());
    if (!GI) return;

    TMap<FName, bool> SavedMap = GI->LoadRepairStatus();

    if (SavedMap.Num() == 0) 
    {
        UE_LOG(LogTemp, Log, TEXT("[RepairActor] 저장된 데이터가 없습니다."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[RepairActor] 게임 인스턴스에서 데이터 로드 중... (%d개)"), SavedMap.Num());

    for (const TPair<FName, bool>& Pair : SavedMap)
    {
        FName Key = Pair.Key;
        bool bIsCompleted = Pair.Value;

        if (RepairStatusMap.Contains(Key))
        {
            RepairStatusMap[Key] = bIsCompleted;
        }
    }

    CompleteRepair();
}

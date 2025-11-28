#include "Interaction/Repair_Actor.h"
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
    Super::BeginPlay();

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
}

bool ARepair_Actor::CheckBodyComplete()
{
    bool bIsBD1Complete = IsRecipeComplete(FName("BD1"));
    bool bIsBD2Complete = IsRecipeComplete(FName("BD2"));

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
    bool bIsEG1Complete = IsRecipeComplete(FName("EG1"));
    bool bIsEG2Complete = IsRecipeComplete(FName("EG2"));

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
    bool bIsST1Complete = IsRecipeComplete(FName("ST1"));
    bool bIsST2Complete = IsRecipeComplete(FName("ST2"));

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
    bool bIsRD1Complete = IsRecipeComplete(FName("RD1"));
    bool bIsRD2Complete = IsRecipeComplete(FName("RD2"));

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
    bool bIsAC1Complete = IsRecipeComplete(FName("AC1"));
    bool bIsAC2Complete = IsRecipeComplete(FName("AC2"));

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

    if (bIsBody && bIsEngine && bIsSteering && bIsRadar && bIsAnchor)
    {
    }
}

void ARepair_Actor::MarkRecipeAsComplete(FName RowName)
{
    if (RepairStatusMap.Contains(RowName))
    {
        RepairStatusMap[RowName] = true;
        
        UE_LOG(LogTemp, Log, TEXT("[RepairActor] 수리 기록됨: %s"), *RowName.ToString());

        CompleteRepair();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[RepairActor] MarkRecipe 실패! Map에 키가 없음: %s"), *RowName.ToString());
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

#include "Interaction/Repair_Actor.h"
#include "Data/ItemDataStruct.h"

bool ARepair_Actor::CheckBodyComplete()
{
    // 테이블이 없으면 실패 처리 및 변수 초기화
    if (!RepairRecipesTable)
    {
       bIsBody = false; 
       return false;
    }

    bool bIsBD1Complete = false;
    bool bIsBD2Complete = false;

    FString ContextString;
    TArray<FRepairRecipeData*> AllRows;
    RepairRecipesTable->GetAllRows<FRepairRecipeData>(ContextString, AllRows);

    for (const FRepairRecipeData* Row : AllRows)
    {
       if (Row)
       {
          if (Row->RecipeName.IsEqual(FName("BD1")) && Row->Complete)
          {
             bIsBD1Complete = true;
          }
          else if (Row->RecipeName.IsEqual(FName("BD2")) && Row->Complete)
          {
             bIsBD2Complete = true;
          }
       }
    }

    // 결과를 멤버 변수에 저장
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
    if (!RepairRecipesTable)
    {
       bIsEngine = false;
       return false;
    }

    bool bIsEG1Complete = false;
    bool bIsEG2Complete = false;

    FString ContextString;
    TArray<FRepairRecipeData*> AllRows;
    RepairRecipesTable->GetAllRows<FRepairRecipeData>(ContextString, AllRows);

    for (const FRepairRecipeData* Row : AllRows)
    {
       if (Row)
       {
          if (Row->RecipeName.IsEqual(FName("EG1")) && Row->Complete)
          {
             bIsEG1Complete = true;
          }
          else if (Row->RecipeName.IsEqual(FName("EG2")) && Row->Complete)
          {
             bIsEG2Complete = true;
          }
       }
    }

    // 결과를 멤버 변수에 저장
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
    if (!RepairRecipesTable)
    {
       bIsSteering = false;
       return false;
    }

    bool bIsST1Complete = false;
    bool bIsST2Complete = false;

    FString ContextString;
    TArray<FRepairRecipeData*> AllRows;
    RepairRecipesTable->GetAllRows<FRepairRecipeData>(ContextString, AllRows);

    for (const FRepairRecipeData* Row : AllRows)
    {
       if (Row)
       {
          if (Row->RecipeName.IsEqual(FName("ST1")) && Row->Complete)
          {
             bIsST1Complete = true;
          }
          else if (Row->RecipeName.IsEqual(FName("ST2")) && Row->Complete)
          {
             bIsST2Complete = true;
          }
       }
    }

    // 결과를 멤버 변수에 저장
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
    if (!RepairRecipesTable)
    {
       bIsRadar = false;
       return false;
    }

    bool bIsRD1Complete = false;
    bool bIsRD2Complete = false;

    FString ContextString;
    TArray<FRepairRecipeData*> AllRows;
    RepairRecipesTable->GetAllRows<FRepairRecipeData>(ContextString, AllRows);

    for (const FRepairRecipeData* Row : AllRows)
    {
       if (Row)
       {
          if (Row->RecipeName.IsEqual(FName("RD1")) && Row->Complete)
          {
             bIsRD1Complete = true;
          }
          else if (Row->RecipeName.IsEqual(FName("RD2")) && Row->Complete)
          {
             bIsRD2Complete = true;
          }
       }
    }

    // 결과를 멤버 변수에 저장
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
    if (!RepairRecipesTable)
    {
       bIsAnchor = false;
       return false;
    }

    bool bIsAC1Complete = false;
    bool bIsAC2Complete = false;

    FString ContextString;
    TArray<FRepairRecipeData*> AllRows;
    RepairRecipesTable->GetAllRows<FRepairRecipeData>(ContextString, AllRows);

    for (const FRepairRecipeData* Row : AllRows)
    {
       if (Row)
       {
          if (Row->RecipeName.IsEqual(FName("AC1")) && Row->Complete)
          {
             bIsAC1Complete = true;
          }
          else if (Row->RecipeName.IsEqual(FName("AC2")) && Row->Complete)
          {
             bIsAC2Complete = true;
          }
       }
    }

    // 결과를 멤버 변수에 저장
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

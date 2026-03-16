#include "Interaction/Repair_Actor.h"
#include "Kismet/GameplayStatics.h"
#include "AdvancedFriendsGameInstance.h"
#include "Data/ItemDataStruct.h"
#include "Net/UnrealNetwork.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

ARepair_Actor::ARepair_Actor()
{
    bReplicates = true;
    bIsBody = false;
    bIsEngine = false;
    bIsSteering = false;
    bIsRadar = false;
    bIsAnchor = false;
}

void ARepair_Actor::OnRep_CompletedRecipes()
{
    if (OnRepairStatusChanged.IsBound())
    {
        OnRepairStatusChanged.Broadcast();
    }
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

void ARepair_Actor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ARepair_Actor, CompletedRecipeNames);
    DOREPLIFETIME(ARepair_Actor, bIsBody);
    DOREPLIFETIME(ARepair_Actor, bIsEngine);
    DOREPLIFETIME(ARepair_Actor, bIsSteering);
    DOREPLIFETIME(ARepair_Actor, bIsRadar);
    DOREPLIFETIME(ARepair_Actor, bIsAnchor);
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

void ARepair_Actor::MarkRecipeAsComplete(FName RecipeName)
{
    if (!HasAuthority()) return;

    if (!CompletedRecipeNames.Contains(RecipeName))
    {
        CompletedRecipeNames.Add(RecipeName);

        if (RepairStatusMap.Contains(RecipeName))
        {
            RepairStatusMap[RecipeName] = true;
            
            UAdvancedFriendsGameInstance* GI = Cast<UAdvancedFriendsGameInstance>(GetGameInstance());
            if (GI)
            {
                GI->SaveRepairStatus(RepairStatusMap);
            }
        }

        if (CheckBodyComplete()) bIsBody = true;
        if (CheckEngineComplete()) bIsEngine = true;
        if (CheckSteeringComplete()) bIsSteering = true;
        if (CheckRadarComplete()) bIsRadar = true;
        if (CheckAnchorComplete()) bIsAnchor = true;
        
        OnRep_CompletedRecipes();
        
        OnRep_RepairStatus();
    }
}

bool ARepair_Actor::IsRecipeComplete(FName TargetName)
{
    if (CompletedRecipeNames.Contains(TargetName))
    {
        return true;
    }

    if (FName* RealRowName = RecipeIDMap.Find(TargetName))
    {
        return CompletedRecipeNames.Contains(*RealRowName);
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
        return;
    }

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

void ARepair_Actor::OnRep_RepairStatus()
{
    UpdateShipVisuals();
}

void ARepair_Actor::SaveData_Implementation(FActorSaveData& OutData)
{
    OutData.ActorID = GUID;
    OutData.Transform = GetActorTransform();
    OutData.ActorClass = GetClass();
	
    FMemoryWriter Writer(OutData.BinaryData, true);
    FObjectAndNameAsStringProxyArchive Ar(Writer, true);
    Ar.ArIsSaveGame = true;

    Serialize(Ar);
}

void ARepair_Actor::LoadData_Implementation(const FActorSaveData& InData)
{
    GUID = InData.ActorID;
    //SetActorTransform(InData.Transform);
	
    FMemoryReader Reader(InData.BinaryData, true);
    FObjectAndNameAsStringProxyArchive Ar(Reader, true);
    Ar.ArIsSaveGame = true;
	
    Serialize(Ar);
	
    UpdateShipVisuals();
    
    ForceNetUpdate();
}

TArray<FName> ARepair_Actor::GetBreakableRecipes() const
{
    return CompletedRecipeNames;
}

bool ARepair_Actor::BreakCompletedRepair(FName RecipeName)
{
	if (!HasAuthority()) return false;

    if (!CompletedRecipeNames.Contains(RecipeName)) return false;

	CompletedRecipeNames.Remove(RecipeName);

    if (RepairStatusMap.Contains(RecipeName))
    {
		RepairStatusMap[RecipeName] = false;
    }

    RefreshRepairProgressState();

    if (UAdvancedFriendsGameInstance* GI = Cast<UAdvancedFriendsGameInstance>(GetGameInstance()))
    {
        GI->SaveRepairStatus(RepairStatusMap);
    }

    OnRep_CompletedRecipes();
    OnRep_RepairStatus();
    UpdateShipVisuals();
    ForceNetUpdate();

    return true;
}

bool ARepair_Actor::BreakRandomCompletedRepair()
{
    if (!HasAuthority())
    {
        return false;
    }

    TArray<FName> BreakableRecipes = GetBreakableRecipes();
    if (BreakableRecipes.Num() == 0)
    {
        return false;
    }

    const int32 RandomIndex = FMath::RandRange(0, BreakableRecipes.Num() - 1);
    return BreakCompletedRepair(BreakableRecipes[RandomIndex]);
}

void ARepair_Actor::RefreshRepairProgressState()
{
    bIsBody = CheckBodyComplete();
    bIsEngine = CheckEngineComplete();
    bIsSteering = CheckSteeringComplete();
    bIsRadar = CheckRadarComplete();
    bIsAnchor = CheckAnchorComplete();
}

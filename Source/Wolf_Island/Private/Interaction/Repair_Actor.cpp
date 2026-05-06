#include "Interaction/Repair_Actor.h"
#include "Kismet/GameplayStatics.h"
#include "AdvancedFriendsGameInstance.h"
#include "Components/BoxComponent.h"
#include "Data/ItemDataStruct.h"
#include "Components/StatusComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Craft/RepairUI.h"
#include "Character/MainPlayer.h"
#include "Games/MainGameState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

ARepair_Actor::ARepair_Actor()
{
    bReplicates = true;
    bIsBody = false;
    bIsEngine = false;
    bIsSteering = false;
    bIsRadar = false;
    bIsAnchor = false;

    USceneComponent* DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
    SetRootComponent(DefaultRoot);

    EscapeReadyVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("EscapeReadyVolume"));
    EscapeReadyVolume->SetupAttachment(RootComponent);
    EscapeReadyVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    EscapeReadyVolume->SetCollisionObjectType(ECC_WorldDynamic);
    EscapeReadyVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    EscapeReadyVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    EscapeReadyVolume->SetGenerateOverlapEvents(true);
}

void ARepair_Actor::Interact_Implementation(AActor* Interactor)
{
    if (!HasAuthority()) return;

    if (bIsBody && bIsEngine && bIsSteering && bIsRadar && bIsAnchor)
    {
        TryEscape(Interactor);
    }
    else
    {
        if (AMainPlayer* Player = Cast<AMainPlayer>(Interactor))
        {
            Player->Client_OpenRepairUI(this);
        }
    }
    
}

void ARepair_Actor::Client_OpenRepairUI_Implementation(class APlayerController* PC)
{
    if (!RepairUIClass || !PC || !PC->IsLocalPlayerController()) return;

    URepairUI* RepairWidget = CreateWidget<URepairUI>(PC, RepairUIClass);
    if (RepairWidget)
    {
        RepairWidget->TargetActor = this;
		
        RepairWidget->AddToViewport();
    }
}

void ARepair_Actor::OnRep_CompletedRecipes()
{
    UWorld* World = GetWorld();
    if (!World || bIsEscaping || IsPendingKillPending() || HasAnyFlags(RF_BeginDestroyed) || World->bIsTearingDown) 
    {
        return;
    }
    
    if (OnRepairStatusChanged.IsBound())
    {
        OnRepairStatusChanged.Broadcast();
    }
}

void ARepair_Actor::BeginPlay()
{

    if (EscapeReadyVolume)
    {
        EscapeReadyVolume->OnComponentBeginOverlap.AddDynamic(this, &ARepair_Actor::OnEscapeVolumeBeginOverlap);
        EscapeReadyVolume->OnComponentEndOverlap.AddDynamic(this, &ARepair_Actor::OnEscapeVolumeEndOverlap);
    }
    
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
                if (!RowData->Sort.IsNone())
                {
                    SortToRecipeRows.FindOrAdd(RowData->Sort).Add(RowName);
                }
            }
        }
    }
    RestoreStateFromGameInstance();

    Super::BeginPlay();
}

void ARepair_Actor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CinematicTimerHandle);
    }
    
    if (EscapeReadyVolume)
    {
        EscapeReadyVolume->OnComponentBeginOverlap.RemoveDynamic(this, &ARepair_Actor::OnEscapeVolumeBeginOverlap);
        EscapeReadyVolume->OnComponentEndOverlap.RemoveDynamic(this, &ARepair_Actor::OnEscapeVolumeEndOverlap);
    }

    Super::EndPlay(EndPlayReason);
}

bool ARepair_Actor::CheckBodyComplete()
{
    bIsBody = IsSortComplete(FName("BD"));
    return bIsBody;
}

bool ARepair_Actor::CheckEngineComplete()
{
    bIsEngine = IsSortComplete(FName("EG"));
    return bIsEngine;
}

bool ARepair_Actor::CheckSteeringComplete()
{
    bIsSteering = IsSortComplete(FName("CT"));
    return bIsSteering;
}

bool ARepair_Actor::CheckRadarComplete()
{
    bIsRadar = IsSortComplete(FName("RD"));
    return bIsRadar;
}

bool ARepair_Actor::CheckAnchorComplete()
{
    bIsAnchor = IsSortComplete(FName("AC"));
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

bool ARepair_Actor::IsSortComplete(FName SortKey)
{
    if (SortKey.IsNone()) return false;

    const TArray<FName>* RowNames = SortToRecipeRows.Find(SortKey);
    if (!RowNames || RowNames->Num() == 0)
    {
        return false;
    }

    for (const FName& RowName : *RowNames)
    {
        if (!IsRecipeComplete(RowName))
        {
            return false;
        }
    }

    return true;
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

void ARepair_Actor::TryEscape(AActor* Interactor)
{
    if (!HasAuthority() || bIsEscaping) return;

    if (!bIsBody || !bIsEngine || !bIsSteering || !bIsRadar || !bIsAnchor)
    {
        return; 
    }

    AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>();
    const bool bIsMulti = GS && GS->IsMulti;

    if (bIsMulti)
    {
        APlayerController* PC = Cast<APlayerController>(Interactor->GetInstigatorController());
        if (!PC || !PC->IsLocalController())
        {
            if (GS)
            {
                FChattingData Notice;
                Notice.Name = TEXT("알림");
                Notice.Message = TEXT("호스트만 탈출을 시작할 수 있습니다.");
                Notice.MessageType = EMessageType::NOTICE;
                GS->AddChattingMessage(Notice);
            }
            return;
        }

        if (!AreAllPlayersInVolume())
        {
            if (GS)
            {
                FChattingData Notice;
                Notice.Name = TEXT("알림");
                Notice.Message = TEXT("모든 생존자가 배 근처에 모여야 탈출할 수 있습니다.");
                Notice.MessageType = EMessageType::NOTICE;
                GS->AddChattingMessage(Notice);
            }
            return;
        }

        if (IsAnyPlayerInfected())
        {
            if (GS)
            {
                FChattingData Notice;
                Notice.Name = TEXT("경고");
                Notice.Message = TEXT("감염된 플레이어가 있어 탈출할 수 없습니다!");
                Notice.MessageType = EMessageType::NOTICE;
                GS->AddChattingMessage(Notice);
            }
            return;
        }
    }

    Multicast_PlayEscapeCinematic();
}

void ARepair_Actor::OnEscapeVolumeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;

    if (AMainPlayer* Player = Cast<AMainPlayer>(OtherActor))
    {
        PlayersInVolume.Add(Player);
    }
}

void ARepair_Actor::OnEscapeVolumeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!HasAuthority() || IsPendingKillPending()) return;

    if (AMainPlayer* Player = Cast<AMainPlayer>(OtherActor))
    {
        PlayersInVolume.Remove(Player);
    }
}

void ARepair_Actor::ExecuteMapTransition()
{
    if (!HasAuthority())
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World || World->bIsTearingDown)
    {
        return;
    }

    UGameplayStatics::OpenLevel(this, FName("/Game/JWY/Maps/StartMap"));
}

bool ARepair_Actor::AreAllPlayersInVolume() const
{
    const AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>();
    if (!GS) return false;

    for (APlayerState* PS : GS->PlayerArray)
    {
        if (!PS) continue;

        AController* Controller = PS->GetOwner<AController>();
        if (!Controller) return false;

        AMainPlayer* PlayerPawn = Cast<AMainPlayer>(Controller->GetPawn());
        if (!PlayerPawn) return false;

        if (!PlayersInVolume.Contains(PlayerPawn))
        {
            return false;
        }
    }

    return true;
}

bool ARepair_Actor::IsAnyPlayerInfected() const
{
    const AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>();
    if (!GS) return false;

    for (APlayerState* PS : GS->PlayerArray)
    {
        if (!PS) continue;

        AController* Controller = PS->GetOwner<AController>();
        if (!Controller) continue;

        AMainPlayer* PlayerPawn = Cast<AMainPlayer>(Controller->GetPawn());
        if (!PlayerPawn) continue;

        if (PlayerPawn->StatusComponent && PlayerPawn->StatusComponent->IsInfected)
        {
            return true;
        }
    }

    return false;
}

void ARepair_Actor::Multicast_PlayEscapeCinematic_Implementation()
{
    bIsEscaping = true;

    OnRepairStatusChanged.Clear();
    
    if (EscapeReadyVolume)
    {
        EscapeReadyVolume->OnComponentBeginOverlap.RemoveAll(this);
        EscapeReadyVolume->OnComponentEndOverlap.RemoveAll(this);
    }

    UWidgetLayoutLibrary::RemoveAllWidgets(this);

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    float CinematicDuration = 5.0f;

    if (PC)
    {
        if (PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, CinematicDuration, FLinearColor::Black, false, true);
        }

        if (APawn* PlayerPawn = PC->GetPawn())
        {
            PlayerPawn->DisableInput(PC);
        }
		
        PC->SetShowMouseCursor(false);
        PC->SetInputMode(FInputModeGameOnly());
    }
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(CinematicTimerHandle, this, &ARepair_Actor::ExecuteMapTransition, CinematicDuration, false);
    }

}

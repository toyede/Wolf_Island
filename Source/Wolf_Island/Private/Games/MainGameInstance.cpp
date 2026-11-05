// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/MainGameInstance.h"

#include "AsyncLoadingScreenLibrary.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Games/MainSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "LoadingScreenSettings.h"
#include "TimerManager.h"
#include "Widgets/Loading/SSeamlessTravelLoadingScreen.h"

namespace MainGameInstanceLoadingScreen
{
	//JWY-다른 뷰포트 위젯(HUD, 메뉴)보다 위에 그리기 위한 Z-Order
	static constexpr int32 ZOrder = 10000;
}

void UMainGameInstance::Init()
{
	Super::Init();

	//JWY-Seamless Travel은 Async Loading Screen의 PreLoadMap 경로를 타지 않아 로딩 화면이 뜨지 않으므로 직접 띄우고 내린다
	SeamlessTravelStartHandle = FWorldDelegates::OnSeamlessTravelStart.AddUObject(this, &UMainGameInstance::HandleSeamlessTravelStart);
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UMainGameInstance::HandlePostLoadMapWithWorld);

	if (GEngine)
	{
		//JWY-이동 실패/연결 끊김 시 로딩 화면이 남지 않도록 내린다
		TravelFailureHandle = GEngine->OnTravelFailure().AddWeakLambda(this,
			[this](UWorld* World, ETravelFailure::Type, const FString&)
			{
				if (!World || World->GetGameInstance() == this)
				{
					HideSeamlessTravelLoadingScreen();
				}
			});
		NetworkFailureHandle = GEngine->OnNetworkFailure().AddWeakLambda(this,
			[this](UWorld* World, UNetDriver*, ENetworkFailure::Type, const FString&)
			{
				if (!World || World->GetGameInstance() == this)
				{
					HideSeamlessTravelLoadingScreen();
				}
			});
	}
}

void UMainGameInstance::Shutdown()
{
	FWorldDelegates::OnSeamlessTravelStart.Remove(SeamlessTravelStartHandle);
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);

	if (GEngine)
	{
		GEngine->OnTravelFailure().Remove(TravelFailureHandle);
		GEngine->OnNetworkFailure().Remove(NetworkFailureHandle);
	}

	HideSeamlessTravelLoadingScreen();

	Super::Shutdown();
}

void UMainGameInstance::HandleSeamlessTravelStart(UWorld* CurrentWorld, const FString& MapName)
{
	//JWY-델리게이트가 전역이라 다른 GameInstance(PIE 다중 클라이언트)의 이동은 무시
	if (!CurrentWorld || CurrentWorld->GetGameInstance() != this)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[LOADING] Seamless travel start: %s"), *MapName);
	ShowSeamlessTravelLoadingScreen();
}

void UMainGameInstance::HandlePostLoadMapWithWorld(UWorld* LoadedWorld)
{
	//JWY-Seamless Travel에서는 최종 목적지 맵의 BeginPlay 이후에만 호출된다(Transition Map에서는 호출되지 않음)
	if (!LoadedWorld || LoadedWorld->GetGameInstance() != this)
	{
		return;
	}

	RequestHideSeamlessTravelLoadingScreen();
}

void UMainGameInstance::ShowSeamlessTravelLoadingScreen()
{
	if (!bShowSeamlessTravelLoadingScreen || SeamlessTravelLoadingWidget.IsValid() || IsDedicatedServerInstance())
	{
		return;
	}

	//JWY-플러그인에서 로딩 화면을 꺼둔 경우(SetEnableLoadingScreen false)에는 같이 띄우지 않는다
	if (!UAsyncLoadingScreenLibrary::GetIsEnableLoadingScreen())
	{
		return;
	}

	UGameViewportClient* ViewportClient = GetGameViewportClient();
	if (!ViewportClient)
	{
		return;
	}

	//JWY-디자인은 Project Settings > Async Loading Screen > Default Loading Screen 값을 그대로 사용한다
	const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>();
	if (!Settings->DefaultLoadingScreen.bShowWidgetOverlay)
	{
		return;
	}

	SeamlessTravelLoadingWidget = SNew(SSeamlessTravelLoadingScreen, Settings->DefaultLoadingScreen, Settings->Classic);
	ViewportClient->AddViewportWidgetContent(SeamlessTravelLoadingWidget.ToSharedRef(), MainGameInstanceLoadingScreen::ZOrder);
	SeamlessTravelLoadingShownTime = FPlatformTime::Seconds();

	FTimerManager& LoadingTimerManager = GetTimerManager();
	LoadingTimerManager.ClearTimer(SeamlessTravelLoadingHideTimerHandle);
	LoadingTimerManager.SetTimer(
		SeamlessTravelLoadingTimeoutHandle,
		FTimerDelegate::CreateUObject(this, &UMainGameInstance::HideSeamlessTravelLoadingScreen),
		FMath::Max(SeamlessTravelLoadingTimeout, 1.0f),
		false);
}

void UMainGameInstance::RequestHideSeamlessTravelLoadingScreen()
{
	if (!SeamlessTravelLoadingWidget.IsValid())
	{
		return;
	}

	//JWY-플러그인과 같은 규칙으로 최소 표시 시간(-1이면 없음)을 지킨 뒤 내린다
	const float MinimumDisplayTime = GetDefault<ULoadingScreenSettings>()->DefaultLoadingScreen.MinimumLoadingScreenDisplayTime;
	const double ElapsedTime = FPlatformTime::Seconds() - SeamlessTravelLoadingShownTime;
	const double RemainingTime = MinimumDisplayTime - ElapsedTime;

	if (MinimumDisplayTime > 0.0f && RemainingTime > 0.0)
	{
		GetTimerManager().SetTimer(
			SeamlessTravelLoadingHideTimerHandle,
			FTimerDelegate::CreateUObject(this, &UMainGameInstance::HideSeamlessTravelLoadingScreen),
			static_cast<float>(RemainingTime),
			false);
		return;
	}

	HideSeamlessTravelLoadingScreen();
}

void UMainGameInstance::HideSeamlessTravelLoadingScreen()
{
	if (TimerManager)
	{
		TimerManager->ClearTimer(SeamlessTravelLoadingHideTimerHandle);
		TimerManager->ClearTimer(SeamlessTravelLoadingTimeoutHandle);
	}

	if (!SeamlessTravelLoadingWidget.IsValid())
	{
		return;
	}

	if (UGameViewportClient* ViewportClient = GetGameViewportClient())
	{
		ViewportClient->RemoveViewportWidgetContent(SeamlessTravelLoadingWidget.ToSharedRef());
	}

	SeamlessTravelLoadingWidget.Reset();
	UE_LOG(LogTemp, Log, TEXT("[LOADING] Seamless travel loading screen hidden"));
}

UMainSaveGame* UMainGameInstance::CreateSaveSlot(FString WorldName, int32 SlotIndex, bool IsMulti)
{
	//멀티 슬롯 생성
	if (IsMulti)
	{
		UMainSaveGame* Save = Cast<UMainSaveGame>(UGameplayStatics::CreateSaveGameObject(UMainSaveGame::StaticClass()));
		Save->WorldName = WorldName;
		Save->SaveUnixTime = FDateTime::UtcNow().ToUnixTimestamp();
		UE_LOG(LogTemp, Warning, TEXT("CREATE MULTI SAVE FILE %s [%s]"), *Save->WorldName, *FDateTime::UtcNow().ToString());
		
		FString SlotName = "M"+FString::Printf(TEXT("%03d"), SlotIndex);
		Save->SlotName = SlotName;
		Save->IsMulti = IsMulti;
		UGameplayStatics::SaveGameToSlot(Save, SlotName, 0);
		UE_LOG(LogTemp, Warning, TEXT("SAVE SLOT AT [%s]"), *SlotName);
		return Save;
	} 
	//싱글 슬롯 생성
	else
	{
		UMainSaveGame* Save = Cast<UMainSaveGame>(UGameplayStatics::CreateSaveGameObject(UMainSaveGame::StaticClass()));
		Save->WorldName = WorldName;
		Save->SaveUnixTime = FDateTime::UtcNow().ToUnixTimestamp();
		UE_LOG(LogTemp, Warning, TEXT("CREATE SINGLE SAVE FILE %s [%s]"), *Save->WorldName, *FDateTime::UtcNow().ToString());
	
		FString SlotName = "S"+FString::Printf(TEXT("%03d"), SlotIndex);
		Save->SlotName = SlotName;
		Save->IsMulti = IsMulti;
		UGameplayStatics::SaveGameToSlot(Save, SlotName, 0);
		UE_LOG(LogTemp, Warning, TEXT("SAVE SLOT AT [%s]"), *SlotName);
		return Save;
	}
}

int32 UMainGameInstance::FindEmptySaveSlotIndex(bool IsMulti)
{
	FString Prefix = IsMulti ? TEXT("M") : TEXT("S");
	
	for (int32 i = 0; i < MaxSlotIndex; i++)
	{
		FString SlotName = Prefix + FString::Printf(TEXT("%03d"), i);

		if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
		{
			return i;
		}
	}
	
	return -1;
}

void UMainGameInstance::CreateLobbySession_Implementation()
{
}

void UMainGameInstance::CreateGameSession_Implementation()
{
}

FString UMainGameInstance::GenerateSessionCode()
{
	FString Code;
	int8 CodeLength = 6;
	FString Characters = "123456789ABCDEFGHIJKLNMOPQRSTUVWXYZ";
	
	for (int i=0; i<CodeLength; i++)
	{
		int8 StartIndex = UKismetMathLibrary::RandomInteger(Characters.Len());
		Code += UKismetStringLibrary::GetSubstring(Characters, StartIndex, 1);
	}
	
	return Code;
}

void UMainGameInstance::CreateSession_Implementation()
{
}

void UMainGameInstance::FindSession_Implementation(const FString& SessionCode)
{
}

void UMainGameInstance::DestroySessionAndReturnToMainMenu_Implementation()
{
	//JWY-BP에서 DestroySession을 아직 연결하지 않아도 최소한 메인 메뉴로 돌아갈 수 있게 하는 fallback
	UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/JWY/Maps/StartMap")));
}

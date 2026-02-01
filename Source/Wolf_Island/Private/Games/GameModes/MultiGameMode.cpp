// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/GameModes/MultiGameMode.h"

#include "GameFramework/PlayerState.h"

void AMultiGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	//채팅 테스트하는 데 플레이어 이름이 너무 길어서 귀염뽀짝 짧은 이름으로 재설정 해주는 개발용 코드
	if (!NewPlayer) return;

	APlayerState* PS = NewPlayer->PlayerState;
	if (!PS) return;

	static int32 Counter = 1;
	static const TArray<FString> Adjs = {
		TEXT("귀여운"), TEXT("빠른"), TEXT("용감한"), TEXT("조용한"),
		TEXT("무거운"), TEXT("느긋한"), TEXT("멍청한"), TEXT("조그만"),
		TEXT("지루한"), TEXT("무서운"), TEXT("재밌는"), TEXT("거대한"),
		TEXT("발정난"), TEXT("옹골진"), TEXT("섹시한"), TEXT("길쭉한"),
	};
	static const TArray<FString> Nouns = {
		TEXT("여우"), TEXT("늑대"), TEXT("토끼"), TEXT("곰"),
		TEXT("고라니"), TEXT("멧돼지"), TEXT("개"), TEXT("고양이"),
		TEXT("닭"), TEXT("땃쥐"), TEXT("까마귀"), TEXT("사슴"),
		TEXT("코끼리"), TEXT("다람쥐"), TEXT("매"), TEXT("살쾡이")
	};

	int32 A = FMath::RandRange(0, Adjs.Num()-1);
	int32 N = FMath::RandRange(0, Nouns.Num()-1);

	PS->SetPlayerName(FString::Printf(TEXT("%s %s%d"), *Adjs[A], *Nouns[N], Counter++));
}

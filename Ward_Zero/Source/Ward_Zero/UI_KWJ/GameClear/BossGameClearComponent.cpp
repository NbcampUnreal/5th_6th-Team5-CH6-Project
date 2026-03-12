// BossGameClearComponent.cpp

#include "UI_KWJ/GameClear/BossGameClearComponent.h"
#include "UI_KWJ/GameClear/GameClearSubsystem.h"
#include "MonsterAI/MonsterAI_CHS/GASMonster/Character/GASBaseMonster.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Ward_Zero.h"

UBossGameClearComponent::UBossGameClearComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBossGameClearComponent::BeginPlay()
{
	Super::BeginPlay();

	// 이 컴포넌트가 붙어있는 액터가 GASBaseMonster인지 확인
	AGASBaseMonster* Boss = Cast<AGASBaseMonster>(GetOwner());
	if (Boss)
	{
		Boss->OnDeathDelegate.AddDynamic(this, &UBossGameClearComponent::OnBossDeath);
		UE_LOG(LogWard_Zero, Log, TEXT("BossGameClearComponent: %s에 사망 델리게이트 바인딩 완료"), *Boss->GetName());
	}
	else
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("BossGameClearComponent: Owner가 GASBaseMonster가 아닙니다!"));
	}
}

void UBossGameClearComponent::OnBossDeath()
{
	UE_LOG(LogWard_Zero, Log, TEXT("보스 사망 → 게임 클리어!"));

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	UGameClearSubsystem* GameClearSys = LP->GetSubsystem<UGameClearSubsystem>();
	if (GameClearSys)
	{
		float PlayTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
		GameClearSys->ShowGameClear(PlayTime);
	}
}

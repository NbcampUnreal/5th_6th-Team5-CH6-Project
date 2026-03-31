// UIManagerSubsystem.cpp

#include "UI_KWJ/UIManagerSubsystem.h"
#include "UI_KWJ/Health/HealthVignetteWidget.h"
#include "UI_KWJ/GameOver/GameOverSubsystem.h"
#include "UI_KWJ/WeaponUI/WeaponUISubsystem.h"
#include "UI_KWJ/HealItem/HealItemSubsystem.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Components/Status/PlayerStatusComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Ward_Zero.h"

void UUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 레벨 로드 완료 시 캐릭터 바인딩 시도
	LevelLoadedHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this, &UUIManagerSubsystem::OnLevelLoaded);

	UE_LOG(LogWard_Zero, Log, TEXT("UIManagerSubsystem 초기화 완료"));
}

void UUIManagerSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(LevelLoadedHandle);
	Super::Deinitialize();
}

// ════════════════════════════════════════════════════════
//  레벨 로드 → 캐릭터 바인딩
// ════════════════════════════════════════════════════════

void UUIManagerSubsystem::OnLevelLoaded(UWorld* LoadedWorld)
{
	// 이전 바인딩 리셋 (레벨 전환 시 캐릭터가 새로 생기니까)
	bBound = false;

	// 한 틱 뒤에 바인딩 시도 (캐릭터 BeginPlay 완료 대기)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick([this]()
		{
			BindToCharacter();
		});
	}
}

void UUIManagerSubsystem::BindToCharacter()
{
	if (bBound) return;

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC) return;

	APrototypeCharacter* Character = Cast<APrototypeCharacter>(PC->GetPawn());
	if (!Character)
	{
		// 캐릭터가 아직 없으면 0.5초 후 재시도
		UE_LOG(LogWard_Zero, Log, TEXT("UIManager: 캐릭터 없음, 0.5초 후 재시도"));
		FTimerHandle RetryHandle;
		World->GetTimerManager().SetTimer(RetryHandle, [this]()
		{
			BindToCharacter();
		}, 0.5f, false);
		return;
	}

	UPlayerStatusComponent* StatusComp = Character->StatusComp;
	if (!StatusComp)
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("UIManager: StatusComp 없음"));
		return;
	}

	// ── HP 비네팅 바인딩 ──
	if (IsValid(HealthVignetteWidget))
	{
		// 위젯은 유효하지만 뷰포트에 없으면 재추가
		if (!HealthVignetteWidget->IsInViewport())
		{
			HealthVignetteWidget->AddToViewport(0);
			HealthVignetteWidget->SetAnchorsInViewport(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			HealthVignetteWidget->SetAlignmentInViewport(FVector2D(0.0f, 0.0f));
			UE_LOG(LogWard_Zero, Log, TEXT("UIManager: HealthVignette 뷰포트 재추가"));
		}
	}
	else
	{
		// 위젯이 무효 또는 null → 새로 생성
		HealthVignetteWidget = nullptr;

		TSubclassOf<UHealthVignetteWidget> VignetteClass = LoadClass<UHealthVignetteWidget>(
			nullptr,
			TEXT("/Game/UI/HP/WBP_HealthVignette.WBP_HealthVignette_C")
		);

		if (VignetteClass)
		{
			HealthVignetteWidget = CreateWidget<UHealthVignetteWidget>(PC, VignetteClass);
			if (HealthVignetteWidget)
			{
				HealthVignetteWidget->AddToViewport(0);
				HealthVignetteWidget->SetAnchorsInViewport(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
				HealthVignetteWidget->SetAlignmentInViewport(FVector2D(0.0f, 0.0f));
				UE_LOG(LogWard_Zero, Log, TEXT("UIManager: HealthVignette 새로 생성"));
			}
		}
		else
		{
			UE_LOG(LogWard_Zero, Warning, TEXT("UIManager: WBP_HealthVignette를 찾을 수 없습니다"));
		}
	}

	// StatusComp → OnHealthChanged → HealthVignetteWidget
	if (HealthVignetteWidget)
	{
		StatusComp->OnHealthChanged.AddDynamic(this, &UUIManagerSubsystem::OnHealthChanged);
	}

	// StatusComp → OnPlayerDied → GameOver
	StatusComp->OnPlayerDied.AddDynamic(this, &UUIManagerSubsystem::OnPlayerDied);

	// ── WeaponUI 탄약 델리게이트 바인딩 ──
	if (UWeaponUISubsystem* WeaponUI = GetLocalPlayer()->GetSubsystem<UWeaponUISubsystem>())
	{
		WeaponUI->BindToStatusComponent(StatusComp);
	}

	// ── HealItem 수량 델리게이트 바인딩 ──
	if (UHealItemSubsystem* HealUI = GetLocalPlayer()->GetSubsystem<UHealItemSubsystem>())
	{
		HealUI->BindToStatusComponent(StatusComp);
		UE_LOG(LogWard_Zero, Log, TEXT("UIManager: HealItemSubsystem 바인딩 호출 완료"));
	}
	else
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("UIManager: HealItemSubsystem을 찾을 수 없음!"));
	}

	bBound = true;
	UE_LOG(LogWard_Zero, Log, TEXT("UIManager: %s 바인딩 완료"), *Character->GetName());
}

// ════════════════════════════════════════════════════════
//  콜백
// ════════════════════════════════════════════════════════

void UUIManagerSubsystem::OnPlayerDied()
{
	UE_LOG(LogWard_Zero, Log, TEXT("UIManager: 플레이어 사망 감지 → 1초 후 게임오버"));

	if (UWorld* World = GetWorld())
	{
		FTimerHandle GameOverTimer;
		TWeakObjectPtr<UUIManagerSubsystem> WeakThis(this);

		World->GetTimerManager().SetTimer(GameOverTimer, [WeakThis]()
		{
			if (UUIManagerSubsystem* StrongThis = WeakThis.Get())
			{
				UGameOverSubsystem* GameOverSys = StrongThis->GetLocalPlayer()->GetSubsystem<UGameOverSubsystem>();
				if (GameOverSys)
				{
					GameOverSys->ShowGameOver();
				}
			}
		}, 1.0f, false);
	}
}

void UUIManagerSubsystem::OnHealthChanged(float Current, float Max)
{
	if (HealthVignetteWidget)
	{
		HealthVignetteWidget->OnHealthChanged(Current, Max);
	}
}

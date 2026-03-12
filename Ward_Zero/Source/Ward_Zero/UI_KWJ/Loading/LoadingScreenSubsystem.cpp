// LoadingScreenSubsystem.cpp

#include "UI_KWJ/Loading/LoadingScreenSubsystem.h"
#include "UI_KWJ/Loading/LoadingWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Ward_Zero.h"

void ULoadingScreenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 레벨 로드 완료 시 자동 숨기기
	LevelLoadedHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this, &ULoadingScreenSubsystem::OnLevelLoaded);

	UE_LOG(LogWard_Zero, Log, TEXT("LoadingScreenSubsystem 초기화 완료"));
}

void ULoadingScreenSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(LevelLoadedHandle);
	Super::Deinitialize();
}

void ULoadingScreenSubsystem::ShowLoading(const FText& Message)
{
	UE_LOG(LogWard_Zero, Warning, TEXT("[Loading] ShowLoading 호출됨"));

	ULoadingWidget* Widget = GetOrCreateWidget();
	if (Widget)
	{
		Widget->SetLoadingMessage(Message);
		Widget->SetVisibility(ESlateVisibility::Visible);
		UE_LOG(LogWard_Zero, Warning, TEXT("[Loading] 로딩 화면 표시 완료: %s"), *Message.ToString());
	}
	else
	{
		UE_LOG(LogWard_Zero, Error, TEXT("[Loading] 위젯 생성 실패 — 로딩 화면 안 나옴"));
	}
}

void ULoadingScreenSubsystem::HideLoading()
{
	if (LoadingWidget)
	{
		LoadingWidget->SetVisibility(ESlateVisibility::Collapsed);
		UE_LOG(LogWard_Zero, Log, TEXT("로딩 화면 숨김"));
	}
}

bool ULoadingScreenSubsystem::IsLoading() const
{
	return LoadingWidget && LoadingWidget->IsVisible();
}

void ULoadingScreenSubsystem::OnLevelLoaded(UWorld* LoadedWorld)
{
	if (!IsLoading()) return;

	UE_LOG(LogWard_Zero, Warning, TEXT("[Loading] OnLevelLoaded → 1.5초 후 숨기기"));

	// 렌더링 안정화 + 화면에 최소 표시 시간 확보
	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<ULoadingScreenSubsystem> WeakThis(this);
		FTimerHandle HideTimer;
		World->GetTimerManager().SetTimer(HideTimer, [WeakThis]()
		{
			if (ULoadingScreenSubsystem* StrongThis = WeakThis.Get())
			{
				StrongThis->HideLoading();
			}
		}, 1.5f, false);
	}
}

ULoadingWidget* ULoadingScreenSubsystem::GetOrCreateWidget()
{
	UE_LOG(LogWard_Zero, Warning, TEXT("[Loading] GetOrCreateWidget 진입"));

	if (IsValid(LoadingWidget))
	{
		if (!LoadingWidget->IsInViewport())
		{
			LoadingWidget->AddToViewport(999);
			LoadingWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		UE_LOG(LogWard_Zero, Warning, TEXT("[Loading] 기존 위젯 재사용"));
		return LoadingWidget;
	}

	if (!LoadingWidgetClass)
	{
		LoadingWidgetClass = LoadClass<ULoadingWidget>(
			nullptr,
			TEXT("/Game/UI/Loading/WBP_Loading.WBP_Loading_C")
		);
	}

	if (!LoadingWidgetClass)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("[Loading] WBP_Loading를 찾을 수 없습니다! 경로 확인 필요"));
		return nullptr;
	}

	UE_LOG(LogWard_Zero, Warning, TEXT("[Loading] WBP_Loading 클래스 로드 성공"));

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("[Loading] PlayerController가 null"));
		return nullptr;
	}

	LoadingWidget = CreateWidget<ULoadingWidget>(PC, LoadingWidgetClass);
	if (LoadingWidget)
	{
		LoadingWidget->AddToViewport(999);
		LoadingWidget->SetVisibility(ESlateVisibility::Collapsed);
		UE_LOG(LogWard_Zero, Warning, TEXT("[Loading] 위젯 생성 + AddToViewport 완료"));
	}
	else
	{
		UE_LOG(LogWard_Zero, Error, TEXT("[Loading] CreateWidget 실패"));
	}

	return LoadingWidget;
}

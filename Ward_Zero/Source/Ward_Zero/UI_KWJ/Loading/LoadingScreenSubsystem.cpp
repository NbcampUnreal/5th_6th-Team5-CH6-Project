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
	ULoadingWidget* Widget = GetOrCreateWidget();
	if (Widget)
	{
		Widget->SetLoadingMessage(Message);
		Widget->SetVisibility(ESlateVisibility::Visible);
		UE_LOG(LogWard_Zero, Log, TEXT("로딩 화면 표시: %s"), *Message.ToString());
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
	// 레벨 로드 완료 → 한 틱 뒤에 로딩 화면 숨기기 (렌더링 안정화 대기)
	if (!IsLoading()) return;

	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<ULoadingScreenSubsystem> WeakThis(this);
		World->GetTimerManager().SetTimerForNextTick([WeakThis]()
		{
			if (ULoadingScreenSubsystem* StrongThis = WeakThis.Get())
			{
				StrongThis->HideLoading();
			}
		});
	}
}

ULoadingWidget* ULoadingScreenSubsystem::GetOrCreateWidget()
{
	if (IsValid(LoadingWidget))
	{
		if (!LoadingWidget->IsInViewport())
		{
			LoadingWidget->AddToViewport(999); // 최상위
			LoadingWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
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
		UE_LOG(LogWard_Zero, Error, TEXT("WBP_Loading를 찾을 수 없습니다!"));
		return nullptr;
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return nullptr;

	LoadingWidget = CreateWidget<ULoadingWidget>(PC, LoadingWidgetClass);
	if (LoadingWidget)
	{
		LoadingWidget->AddToViewport(999); // 모든 UI 위에
		LoadingWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	return LoadingWidget;
}

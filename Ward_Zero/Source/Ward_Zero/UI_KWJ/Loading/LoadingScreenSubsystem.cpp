// LoadingScreenSubsystem.cpp

#include "UI_KWJ/Loading/LoadingScreenSubsystem.h"
#include "UI_KWJ/Loading/LoadingWidget.h"
#include "MoviePlayer.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Blueprint/UserWidget.h"
#include "Ward_Zero.h"

void ULoadingScreenSubsystem::ShowLoading(const FText& Message)
{
	if (bIsLoading) return;

	IGameMoviePlayer* MoviePlayer = GetMoviePlayer();
	if (!MoviePlayer) 
	{
		UE_LOG(LogWard_Zero, Error, TEXT("[Loading] MoviePlayer를 가져올 수 없습니다"));
		return;
	}

	// 로딩 위젯 생성
	TSubclassOf<ULoadingWidget> LoadingClass = LoadClass<ULoadingWidget>(
		nullptr,
		TEXT("/Game/UI/Loading/WBP_Loading.WBP_Loading_C")
	);

	if (!LoadingClass)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("[Loading] WBP_Loading를 찾을 수 없습니다"));
		return;
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return;

	ULoadingWidget* LoadingWidget = CreateWidget<ULoadingWidget>(PC, LoadingClass);
	if (!LoadingWidget)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("[Loading] 위젯 생성 실패"));
		return;
	}

	LoadingWidget->SetLoadingMessage(Message);

	// MoviePlayer에 로딩 화면 등록
	FLoadingScreenAttributes LoadingScreen;
	LoadingScreen.WidgetLoadingScreen = LoadingWidget->TakeWidget(); // Slate 위젯으로 변환
	LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;          // 로딩 끝나면 자동 해제
	LoadingScreen.bMoviesAreSkippable = false;                       // 스킵 불가
	LoadingScreen.bWaitForManualStop = false;                        // 수동 정지 대기 안 함
	LoadingScreen.MinimumLoadingScreenDisplayTime = 1.5f;            // 최소 1.5초 표시

	MoviePlayer->SetupLoadingScreen(LoadingScreen);

	bIsLoading = true;
	UE_LOG(LogWard_Zero, Log, TEXT("[Loading] MoviePlayer 로딩 화면 설정 완료: %s"), *Message.ToString());
}

void ULoadingScreenSubsystem::HideLoading()
{
	if (!bIsLoading) return;

	IGameMoviePlayer* MoviePlayer = GetMoviePlayer();
	if (MoviePlayer)
	{
		MoviePlayer->StopMovie();
	}

	bIsLoading = false;
	UE_LOG(LogWard_Zero, Log, TEXT("[Loading] 로딩 화면 수동 해제"));
}

bool ULoadingScreenSubsystem::IsLoading() const
{
	return bIsLoading;
}

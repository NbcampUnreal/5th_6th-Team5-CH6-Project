// SaveSubsystem.cpp

#include "UI_KWJ/Save/SaveSubsystem.h"
#include "UI_KWJ/Save/WardSaveGame.h"
#include "UI_KWJ/Save/SaveWidget.h"
#include "UI_KWJ/Save/LoadWidget.h"
#include "UI_KWJ/GameOver/GameOverSubsystem.h"
#include "UI_KWJ/Loading/LoadingScreenSubsystem.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Components/Status/PlayerStatusComponent.h"
#include "Character/Components/Combat/PlayerCombatComponent.h"
#include "Weapon/Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Texture2D.h"
#include "Engine/GameViewportClient.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "RenderingThread.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Ward_Zero.h"

const FString USaveSubsystem::SavePrefix = TEXT("WardZero_");

void USaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 레벨 로드 완료 시 PendingSaveData 적용 콜백 등록
	// AddUObject: this가 파괴되면 콜백 자동 무시 (AddLambda보다 안전)
	OnLevelLoadedHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this, &USaveSubsystem::OnLevelLoaded);

	UE_LOG(LogWard_Zero, Log, TEXT("SaveSubsystem 초기화 완료"));
}

void USaveSubsystem::OnLevelLoaded(UWorld* LoadedWorld)
{
	if (!PendingSaveData) return;

	UWardSaveGame* DataToApply = PendingSaveData;
	PendingSaveData = nullptr;

	// 한 틱 뒤에 적용 (레벨 액터들이 완전히 초기화된 후)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick([this, DataToApply]()
		{
			ApplyGameState(DataToApply);
			UE_LOG(LogWard_Zero, Log, TEXT("레벨 전환 후 세이브 데이터 적용 완료"));
		});
	}
}

void USaveSubsystem::Deinitialize()
{
	// 콜백 해제 — 서브시스템 파괴 후 콜백이 호출되면 크래시 발생
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(OnLevelLoadedHandle);

	Super::Deinitialize();
}

// ════════════════════════════════════════════════════════
//  저장
// ════════════════════════════════════════════════════════

void USaveSubsystem::SaveGame(const FString& SlotName)
{
	UWardSaveGame* SaveData = CollectCurrentGameState();
	if (!SaveData)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("세이브 데이터 수집 실패"));
		return;
	}

	FString FinalSlotName = SlotName.IsEmpty() ? GenerateSlotName() : SlotName;


	SaveData->PlayTimeSeconds = UGameplayStatics::GetRealTimeSeconds(GetWorld());

	UGameplayStatics::SaveGameToSlot(SaveData, SavePrefix + FinalSlotName, 0);


	SaveData->DisplayName = FinalSlotName;

	// 캐시된 스크린샷 사용 (UI 열기 전에 미리 캡처)
	if (CachedScreenshotData.Num() > 0)
	{
		SaveData->ScreenshotData = CachedScreenshotData;
		SaveData->ScreenshotWidth = CachedScreenshotWidth;
		SaveData->ScreenshotHeight = CachedScreenshotHeight;
	}
	else
	{
		CaptureScreenshot(SaveData->ScreenshotData, SaveData->ScreenshotWidth, SaveData->ScreenshotHeight);
	}

	if (UGameplayStatics::SaveGameToSlot(SaveData, SavePrefix + FinalSlotName, 0))
	{
		LastSaveSlotName = FinalSlotName;
		UE_LOG(LogWard_Zero, Log, TEXT("세이브 성공: %s"), *FinalSlotName);
	}
	else
	{
		UE_LOG(LogWard_Zero, Error, TEXT("세이브 실패: %s"), *FinalSlotName);
	}
}

// ════════════════════════════════════════════════════════
//  로드
// ════════════════════════════════════════════════════════

bool USaveSubsystem::LoadGame(const FString& SlotName)
{
	FString FullSlotName = SavePrefix + SlotName;

	if (!UGameplayStatics::DoesSaveGameExist(FullSlotName, 0))
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("세이브 파일 없음: %s"), *SlotName);
		return false;
	}

	UWardSaveGame* SaveData = Cast<UWardSaveGame>(
		UGameplayStatics::LoadGameFromSlot(FullSlotName, 0)
	);

	if (!SaveData)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("세이브 로드 실패: %s"), *SlotName);
		return false;
	}

	// GameOver UI가 열려 있으면 닫기
	if (UGameOverSubsystem* GameOverSys = GetLocalPlayer()->GetSubsystem<UGameOverSubsystem>())
	{
		if (GameOverSys->IsGameOver())
		{
			GameOverSys->HideGameOver();
		}
	}

	// 로드 시 Save/Load UI 모두 닫기
	HideSaveUI();
	HideLoadUI();

	// 로딩 화면 표시
	if (ULoadingScreenSubsystem* LoadingSys = GetLocalPlayer()->GetSubsystem<ULoadingScreenSubsystem>())
	{
		LoadingSys->ShowLoading(FText::FromString(TEXT("Loading...")));
	}

	FName CurrentLevel = FName(*UGameplayStatics::GetCurrentLevelName(GetWorld()));
	if (SaveData->CurrentLevelName != NAME_None && SaveData->CurrentLevelName != CurrentLevel)
	{
		// 다른 레벨이면 전환 후 OnLevelLoaded에서 ApplyGameState 호출
		UE_LOG(LogWard_Zero, Log, TEXT("레벨 전환: %s → %s"),
			*CurrentLevel.ToString(), *SaveData->CurrentLevelName.ToString());

		PendingSaveData = SaveData;

		FString TravelURL = SaveData->CurrentLevelName.ToString();
		GetWorld()->ServerTravel(TravelURL, true);
	}
	else
	{
		// 같은 레벨이면 바로 적용
		ApplyGameState(SaveData);

		// 로딩 화면 숨기기
		if (ULoadingScreenSubsystem* LoadingSys = GetLocalPlayer()->GetSubsystem<ULoadingScreenSubsystem>())
		{
			LoadingSys->HideLoading();
		}

		UE_LOG(LogWard_Zero, Log, TEXT("세이브 로드 완료: %s"), *SlotName);
	}

	return true;
}

bool USaveSubsystem::LoadLastSave()
{
	if (LastSaveSlotName.IsEmpty())
	{
		// 마지막 세이브가 없으면 목록에서 최신 것을 찾기
		TArray<FSaveFileInfo> Saves = GetSaveFileList();
		if (Saves.Num() > 0)
		{
			return LoadGame(Saves[0].SlotName);
		}

		UE_LOG(LogWard_Zero, Warning, TEXT("로드할 세이브가 없습니다"));
		return false;
	}

	return LoadGame(LastSaveSlotName);
}

// ════════════════════════════════════════════════════════
//  삭제
// ════════════════════════════════════════════════════════

bool USaveSubsystem::DeleteSave(const FString& SlotName)
{
	FString FullSlotName = SavePrefix + SlotName;

	if (UGameplayStatics::DeleteGameInSlot(FullSlotName, 0))
	{
		UE_LOG(LogWard_Zero, Log, TEXT("세이브 삭제: %s"), *SlotName);
		return true;
	}

	UE_LOG(LogWard_Zero, Warning, TEXT("세이브 삭제 실패: %s"), *SlotName);
	return false;
}

// ════════════════════════════════════════════════════════
//  목록
// ════════════════════════════════════════════════════════

TArray<FSaveFileInfo> USaveSubsystem::GetSaveFileList()
{
	TArray<FSaveFileInfo> Result;

	FString SaveDir = FPaths::ProjectSavedDir() / TEXT("SaveGames");
	TArray<FString> FoundFiles;
	IFileManager::Get().FindFiles(FoundFiles, *SaveDir, TEXT(".sav"));

	for (const FString& FileName : FoundFiles)
	{
		FString BaseName = FPaths::GetBaseFilename(FileName);
		if (!BaseName.StartsWith(SavePrefix))
		{
			continue;
		}

		UWardSaveGame* SaveData = Cast<UWardSaveGame>(
			UGameplayStatics::LoadGameFromSlot(BaseName, 0)
		);

		if (SaveData)
		{
			FSaveFileInfo Info;
			Info.SlotName = BaseName.RightChop(SavePrefix.Len());
			Info.DisplayName = SaveData->DisplayName;
			Info.SaveDateTime = SaveData->SaveDateTime;
			Info.PlayTimeSeconds = SaveData->PlayTimeSeconds;
			Info.LevelName = SaveData->CurrentLevelName;

			if (SaveData->ScreenshotData.Num() > 0)
			{
				Info.Thumbnail = CreateThumbnailFromPNG(
					SaveData->ScreenshotData,
					SaveData->ScreenshotWidth,
					SaveData->ScreenshotHeight
				);
			}

			Result.Add(Info);
		}
	}

	Result.Sort([](const FSaveFileInfo& A, const FSaveFileInfo& B)
	{
		return A.SaveDateTime > B.SaveDateTime;
	});

	return Result;
}

// ════════════════════════════════════════════════════════
//  게임 상태 수집
// ════════════════════════════════════════════════════════

UWardSaveGame* USaveSubsystem::CollectCurrentGameState()
{
	UWardSaveGame* SaveData = NewObject<UWardSaveGame>();

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return nullptr;

	APrototypeCharacter* Character = Cast<APrototypeCharacter>(PC->GetPawn());
	if (!Character) return nullptr;

	// 위치 & 회전
	SaveData->PlayerLocation = Character->GetActorLocation();
	SaveData->PlayerRotation = PC->GetControlRotation();

	// 체력 & 스태미나
	UPlayerStatusComponent* Status = Character->FindComponentByClass<UPlayerStatusComponent>();
	if (Status)
	{
		SaveData->CurrentHealth = Status->CurrHealth;
		SaveData->MaxHealth = Status->MaxHealth;
		SaveData->CurrentStamina = Status->CurrStamina;
		SaveData->MaxStamina = Status->MaxStamina;
		SaveData->bIsExhausted = Status->bIsExhausted;
	}

	// 무기 & 탄약
	UPlayerCombatComponent* Combat = Character->FindComponentByClass<UPlayerCombatComponent>();
	if (Combat)
	{
		AWeapon* Weapon = Combat->GetEquippedWeapon();
		SaveData->bIsWeaponEquipped = (Weapon != nullptr);
		if (Weapon)
		{
			SaveData->CurrentAmmo = Weapon->GetCurrentAmmo();
			SaveData->MaxAmmoCapacity = Weapon->GetMaxCapacity();
		}
	}

	// 손전등
	SaveData->bIsFlashLightOn = Character->GetIsUseFlashLight();

	// 레벨 & 시간
	SaveData->CurrentLevelName = FName(*UGameplayStatics::GetCurrentLevelName(GetWorld()));
	SaveData->SaveDateTime = FDateTime::Now();
	SaveData->PlayTimeSeconds = 0.f; // TODO: 별도 추적

	return SaveData;
}

// ════════════════════════════════════════════════════════
//  게임 상태 적용
// ════════════════════════════════════════════════════════

void USaveSubsystem::ApplyGameState(UWardSaveGame* SaveData)
{
	if (!SaveData) return;

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC) return;

	APrototypeCharacter* Character = Cast<APrototypeCharacter>(PC->GetPawn());
	if (!Character) return;

	// 사망 상태면 캐릭터 Revive() 호출 (래그돌/입력/콜리전 복원은 캐릭터가 처리)
	if (Character->StatusComp && Character->StatusComp->IsDead())
	{
		Character->Revive();
	}

	// 일시정지 해제 (게임오버에서 일시정지 중일 수 있음)
	UGameplayStatics::SetGamePaused(World, false);

	// 위치 & 회전
	Character->SetActorLocation(SaveData->PlayerLocation);
	PC->SetControlRotation(SaveData->PlayerRotation);

	// 체력 & 스태미나
	UPlayerStatusComponent* Status = Character->FindComponentByClass<UPlayerStatusComponent>();
	if (Status)
	{
		Status->CurrHealth = SaveData->CurrentHealth;
		Status->MaxHealth = SaveData->MaxHealth;
		Status->bIsDead = false;
		Status->CurrStamina = SaveData->CurrentStamina;
		Status->MaxStamina = SaveData->MaxStamina;
		Status->bIsExhausted = SaveData->bIsExhausted;

		// HP 비네팅 즉시 갱신
		Status->OnHealthChanged.Broadcast(Status->CurrHealth, Status->MaxHealth);
	}

	// TODO: 무기 장착 상태 & 탄약 복원
	if (SaveData->bIsWeaponEquipped)
	{
		UE_LOG(LogWard_Zero, Log, TEXT("무기 복원 필요 — 탄약: %d/%d"),
			SaveData->CurrentAmmo, SaveData->MaxAmmoCapacity);
	}

	// TODO: 손전등 상태 복원
	if (SaveData->bIsFlashLightOn)
	{
		UE_LOG(LogWard_Zero, Log, TEXT("손전등 ON 복원 필요"));
	}

	UE_LOG(LogWard_Zero, Log, TEXT("게임 상태 복원 — 위치: %s, HP: %.0f/%.0f, 스태미나: %.0f/%.0f"),
		*SaveData->PlayerLocation.ToString(),
		SaveData->CurrentHealth, SaveData->MaxHealth,
		SaveData->CurrentStamina, SaveData->MaxStamina);
}

// ════════════════════════════════════════════════════════
//  스크린샷
// ════════════════════════════════════════════════════════

void USaveSubsystem::CaptureScreenshot(TArray<uint8>& OutPNGData, int32& OutWidth, int32& OutHeight)
{
	OutPNGData.Empty();
	OutWidth = 0;
	OutHeight = 0;

	UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
	if (!ViewportClient)
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("스크린샷: ViewportClient 없음"));
		return;
	}

	FViewport* Viewport = ViewportClient->Viewport;
	if (!Viewport)
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("스크린샷: Viewport 없음"));
		return;
	}

	TArray<FColor> Bitmap;
	FIntPoint Size = Viewport->GetSizeXY();

	if (Size.X == 0 || Size.Y == 0)
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("스크린샷: 뷰포트 크기 0 (%d x %d)"), Size.X, Size.Y);
		return;
	}

	// 렌더링 완료 대기 후 ReadPixels
	FlushRenderingCommands();

	bool bSuccess = Viewport->ReadPixels(Bitmap);
	if (!bSuccess || Bitmap.Num() == 0)
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("스크린샷: ReadPixels 실패 (bSuccess=%d, Num=%d)"), bSuccess, Bitmap.Num());
		return;
	}

	// 픽셀 데이터 검증 - 전부 검정인지 확인
	int32 NonBlackCount = 0;
	for (int32 i = 0; i < FMath::Min(Bitmap.Num(), 1000); i++)
	{
		if (Bitmap[i].R > 0 || Bitmap[i].G > 0 || Bitmap[i].B > 0)
		{
			NonBlackCount++;
		}
	}
	UE_LOG(LogWard_Zero, Log, TEXT("스크린샷: ReadPixels 성공 (%d x %d, %d pixels, 비검정=%d/1000)"), 
		Size.X, Size.Y, Bitmap.Num(), NonBlackCount);

	int32 ThumbWidth = 320;
	int32 ThumbHeight = 180;
	TArray<FColor> ResizedBitmap;
	ResizedBitmap.SetNum(ThumbWidth * ThumbHeight);

	float XRatio = static_cast<float>(Size.X) / ThumbWidth;
	float YRatio = static_cast<float>(Size.Y) / ThumbHeight;

	for (int32 Y = 0; Y < ThumbHeight; Y++)
	{
		for (int32 X = 0; X < ThumbWidth; X++)
		{
			int32 SrcX = FMath::Clamp(static_cast<int32>(X * XRatio), 0, Size.X - 1);
			int32 SrcY = FMath::Clamp(static_cast<int32>(Y * YRatio), 0, Size.Y - 1);
			FColor Pixel = Bitmap[SrcY * Size.X + SrcX];
			Pixel.A = 255; // 알파 강제 불투명
			ResizedBitmap[Y * ThumbWidth + X] = Pixel;
		}
	}

	TArray64<uint8> PNGData64;
	FImageUtils::PNGCompressImageArray(ThumbWidth, ThumbHeight, ResizedBitmap, PNGData64);
	OutPNGData.SetNum(PNGData64.Num());
	FMemory::Memcpy(OutPNGData.GetData(), PNGData64.GetData(), PNGData64.Num());
	OutWidth = ThumbWidth;
	OutHeight = ThumbHeight;

	UE_LOG(LogWard_Zero, Log, TEXT("스크린샷 캡처 완료: %d bytes PNG"), OutPNGData.Num());

	// 디버그: PNG를 파일로 저장해서 확인
	FString DebugPath = FPaths::ProjectSavedDir() / TEXT("DebugScreenshot.png");
	FFileHelper::SaveArrayToFile(OutPNGData, *DebugPath);
	UE_LOG(LogWard_Zero, Log, TEXT("디버그 PNG 저장: %s"), *DebugPath);
}

UTexture2D* USaveSubsystem::CreateThumbnailFromPNG(const TArray<uint8>& PNGData, int32 Width, int32 Height)
{
	if (PNGData.Num() == 0)
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("썸네일: PNG 데이터 없음"));
		return nullptr;
	}

	UTexture2D* Texture = FImageUtils::ImportBufferAsTexture2D(PNGData);
	if (!Texture)
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("썸네일: ImportBufferAsTexture2D 실패"));
		return nullptr;
	}

	UE_LOG(LogWard_Zero, Log, TEXT("썸네일 생성 성공: %dx%d"), Texture->GetSizeX(), Texture->GetSizeY());
	return Texture;
}

FString USaveSubsystem::GenerateSlotName() const
{
	return FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
}

// ════════════════════════════════════════════════════════
//  세이브 UI
// ════════════════════════════════════════════════════════

void USaveSubsystem::ShowSaveUI()
{
	// UI가 뜨기 전에 스크린샷 캐시
	CaptureScreenshot(CachedScreenshotData, CachedScreenshotWidth, CachedScreenshotHeight);

	USaveWidget* Widget = GetOrCreateSaveUI();
	if (Widget)
	{
		Widget->RefreshSaveList();
		Widget->SetVisibility(ESlateVisibility::Visible);
		Widget->SetKeyboardFocus();

		APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
		if (PC)
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(Widget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(true);
		}
	}
}

void USaveSubsystem::HideSaveUI()
{
	if (SaveWidget)
	{
		SaveWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}
}

bool USaveSubsystem::IsSaveUIOpen() const
{
	return SaveWidget && SaveWidget->IsVisible();
}

// ════════════════════════════════════════════════════════
//  불러오기 UI (ESC / 게임오버용)
// ════════════════════════════════════════════════════════

void USaveSubsystem::ShowLoadUI(bool bFromGameOver)
{
	ULoadWidget* Widget = GetOrCreateLoadUI();
	if (Widget)
	{
		Widget->RefreshSaveList();
		Widget->SetCloseButtonVisible(!bFromGameOver); // 게임오버에서 열면 나가기 숨김
		Widget->SetVisibility(ESlateVisibility::Visible);
		Widget->SetKeyboardFocus();

		APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
		if (PC)
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(Widget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(true);
		}
	}
}

void USaveSubsystem::HideLoadUI()
{
	if (LoadWidgetInstance)
	{
		LoadWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}
}

bool USaveSubsystem::IsLoadUIOpen() const
{
	return LoadWidgetInstance && LoadWidgetInstance->IsVisible();
}

// ════════════════════════════════════════════════════════
//  위젯 생성
// ════════════════════════════════════════════════════════

USaveWidget* USaveSubsystem::GetOrCreateSaveUI()
{
	if (IsValid(SaveWidget))
	{
		if (!SaveWidget->IsInViewport())
		{
			SaveWidget->AddToViewport(150);
			SaveWidget->SetAnchorsInViewport(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			SaveWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		return SaveWidget;
	}

	SaveWidget = nullptr;

	if (!SaveWidgetClass)
	{
		SaveWidgetClass = LoadClass<USaveWidget>(
			nullptr,
			TEXT("/Game/UI/Save/WBP_SaveMenu.WBP_SaveMenu_C")
		);
	}

	if (!SaveWidgetClass)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("WBP_SaveMenu를 찾을 수 없습니다!"));
		return nullptr;
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return nullptr;

	SaveWidget = CreateWidget<USaveWidget>(PC, SaveWidgetClass);
	if (SaveWidget)
	{
		SaveWidget->AddToViewport(150);
		SaveWidget->SetAnchorsInViewport(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		SaveWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	return SaveWidget;
}

ULoadWidget* USaveSubsystem::GetOrCreateLoadUI()
{
	if (IsValid(LoadWidgetInstance))
	{
		if (!LoadWidgetInstance->IsInViewport())
		{
			LoadWidgetInstance->AddToViewport(150);
			LoadWidgetInstance->SetAnchorsInViewport(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			LoadWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		}
		return LoadWidgetInstance;
	}

	LoadWidgetInstance = nullptr;

	if (!LoadWidgetClass)
	{
		LoadWidgetClass = LoadClass<ULoadWidget>(
			nullptr,
			TEXT("/Game/UI/Save/WBP_LoadMenu.WBP_LoadMenu_C")
		);
	}

	if (!LoadWidgetClass)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("WBP_LoadMenu를 찾을 수 없습니다!"));
		return nullptr;
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return nullptr;

	LoadWidgetInstance = CreateWidget<ULoadWidget>(PC, LoadWidgetClass);
	if (LoadWidgetInstance)
	{
		LoadWidgetInstance->AddToViewport(150);
		LoadWidgetInstance->SetAnchorsInViewport(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		LoadWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}

	return LoadWidgetInstance;
}

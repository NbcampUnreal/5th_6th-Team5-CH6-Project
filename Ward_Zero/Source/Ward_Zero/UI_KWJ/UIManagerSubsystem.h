// UIManagerSubsystem.h
// UI 허브 — 캐릭터 이벤트 감지 → 각 UI 서브시스템/위젯에 전달
// 캐릭터 코드에 UI 관련 코드를 넣지 않아도 되게 해줌

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UIManagerSubsystem.generated.h"

class UHealthVignetteWidget;

UCLASS()
class WARD_ZERO_API UUIManagerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:

	// ── 레벨 로드 → 캐릭터 바인딩 ──

	void OnLevelLoaded(UWorld* LoadedWorld);
	FDelegateHandle LevelLoadedHandle;

	/** 캐릭터를 찾아서 델리게이트 바인딩 */
	void BindToCharacter();

	// ── 콜백 ──

	UFUNCTION()
	void OnPlayerDied();

	UFUNCTION()
	void OnHealthChanged(float Current, float Max);

	// ── HP 비네팅 ──

	UPROPERTY()
	UHealthVignetteWidget* HealthVignetteWidget;

	/** 바인딩 완료 여부 (중복 바인딩 방지) */
	bool bBound = false;
};

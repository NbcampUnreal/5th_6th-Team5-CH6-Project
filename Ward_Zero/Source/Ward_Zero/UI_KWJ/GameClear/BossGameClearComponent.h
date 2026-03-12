// BossGameClearComponent.h
// 보스 몬스터에 부착하면 사망 시 GameClearSubsystem 호출
// 사용법: ABP_Boss_ESM 블루프린트에서 이 컴포넌트를 추가하면 끝

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BossGameClearComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WARD_ZERO_API UBossGameClearComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBossGameClearComponent();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnBossDeath();
};

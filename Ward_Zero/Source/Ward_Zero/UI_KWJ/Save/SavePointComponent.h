// SavePointComponent.h
// 세이브 포인트 컴포넌트
//
// 사용법 1 (태그): 레벨의 아무 액터에 Tag "SavePoint" 추가
// 사용법 2 (컴포넌트): 액터에 USavePointComponent 추가
//
// InteractionComponent가 라인 트레이스로 감지 → E키 → 세이브 UI

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SavePointComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WARD_ZERO_API USavePointComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USavePointComponent();

	/** 세이브 UI 열기 */
	void ActivateSavePoint(class APrototypeCharacter* Player);
};

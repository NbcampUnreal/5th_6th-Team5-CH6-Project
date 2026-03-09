// InteractionComponent.h
// 라인 트레이스 기반 상호작용 감지 + E키 프롬프트 UI
// 캐릭터에 부착하여 매 틱 전방 트레이스, 상호작용 가능 액터 감지 시 위젯 표시

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class UWidgetComponent;
class UUserWidget;

// 상호작용 가능한 액터가 구현하는 인터페이스 (기존 IInteract, IInteractionBase 모두 지원)
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WARD_ZERO_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** E키 눌렀을 때 호출 — 현재 포커스된 액터에 상호작용 실행 */
	void TryInteract();

	/** 현재 바라보고 있는 상호작용 가능 액터 */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetFocusedActor() const { return FocusedActor; }

protected:

	// ══════════════════════════════════════════
	//  설정
	// ══════════════════════════════════════════

	/** 상호작용 가능 최대 거리 */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionRange = 250.0f;

	/** 트레이스 채널 */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	/** E키 프롬프트 위젯 클래스 (WBP_InteractPrompt) */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction|UI")
	TSubclassOf<UUserWidget> PromptWidgetClass;

	/** 프롬프트가 액터 위로 얼마나 올라갈지 */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction|UI")
	float PromptHeightOffset = 60.0f;

private:

	// ══════════════════════════════════════════
	//  내부 상태
	// ══════════════════════════════════════════

	/** 현재 포커스된 액터 */
	UPROPERTY()
	AActor* FocusedActor;

	/** 프롬프트 위젯 컴포넌트 (동적 생성) */
	UPROPERTY()
	UWidgetComponent* PromptWidgetComp;

	/** 라인 트레이스 수행 */
	void PerformTrace();

	/** 포커스 대상 변경 처리 */
	void SetFocusedActor(AActor* NewActor);

	/** 프롬프트 위치 업데이트 */
	void UpdatePromptLocation();

	/** 액터가 상호작용 가능한지 체크 (IInteract 또는 IInteractionBase) */
	bool IsInteractable(AActor* Actor) const;

	/** 상호작용 실행 */
	void ExecuteInteraction(AActor* Actor);
};

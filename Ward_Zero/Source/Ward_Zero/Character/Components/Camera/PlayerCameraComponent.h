#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerCameraComponent.generated.h"

class USpringArmComponent;
class UCameraComponent;
class APrototypeCharacter;
class UCameraData;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WARD_ZERO_API UPlayerCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerCameraComponent();

	// 초기화: 캐릭터에서 카메라 컴포넌트들을 넘겨받음
	void Initialize(USpringArmComponent* InBoom, UCameraComponent* InCamera, UCameraData* InData);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// 매 틱 호출될 업데이트 로직
	void UpdateCamera(float DeltaTime);

protected:
	UPROPERTY() TObjectPtr<APrototypeCharacter> OwnerCharacter;
	UPROPERTY() TObjectPtr<USpringArmComponent> CameraBoom;
	UPROPERTY() TObjectPtr<UCameraComponent> MainCamera;
	UPROPERTY() TObjectPtr<UCameraData> CameraData;

private:
	float BobTime = 0.0f;
	FVector OriginalSocketOffset;
	FVector OriginalTargetOffset;
	FRotator LastSwayRot = FRotator::ZeroRotator;

	bool bWasCrouched = false;

	float CurrentSlopeOffset = 0.0f;

	bool bIsInVent = false;

	UPROPERTY()
	TObjectPtr<class UPlayerCombatComponent> CachedCombatComp; 
};

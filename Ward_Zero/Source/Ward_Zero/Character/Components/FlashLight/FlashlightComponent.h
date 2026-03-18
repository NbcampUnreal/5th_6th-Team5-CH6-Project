#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Animation/Interface/PlayerAnimInterface.h"
#include "FlashlightComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WARD_ZERO_API UFlashlightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFlashlightComponent();

	void ToggleFlashlight();
	void UpdateFlashlight(float DeltaTime);
	float CalculateFocusAlpha(class USpotLightComponent* Light, float MaxDist);

	bool IsUsingFlashlight() const { return bIsUseFlashlight; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(EditAnywhere, Category = "Flashlight")
	TSubclassOf<class AFlashLight> FlashLightClass;

	UPROPERTY(EditDefaultsOnly, Category = "Flashlight")
	TObjectPtr<class UFlashLightData> DefaultFlashlightData;

public:
	void SetFlashlightOff();

private:
	bool bIsUseFlashlight = false;

	UPROPERTY()
	TObjectPtr<class AFlashLight> FlashLightActor;

	UPROPERTY()
	class USpotLightComponent* BodyRunLight;

public:
	UPROPERTY(EditAnywhere, Category = "FlashLight|Montage")
	TObjectPtr<UAnimMontage> RaiseLightMontage;

	UPROPERTY(EditAnywhere, Category = "FlashLight|Montage")
	TObjectPtr<UAnimMontage> LowerLightMontage;

public:
	// 0(걷기/조준) ~ 1(달리기) 사이를 부드럽게 오가는 변수
	float SprintInterpAlpha = 0.0f;

	// 캐싱용 변수 - 매번 Cast 하지 않게 최적화 
	UPROPERTY()
	TObjectPtr<class UPlayerCombatComponent> CachedCombatComp;

	UPROPERTY()
	TObjectPtr<class AWeapon> LastEquippedWeapon; 
};

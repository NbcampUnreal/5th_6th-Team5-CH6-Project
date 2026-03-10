// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlashlightComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WARD_ZERO_API UFlashlightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFlashlightComponent();

	void ToggleFlashlight();
	void UpdateFlashlight(float DeltaTime);

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
};

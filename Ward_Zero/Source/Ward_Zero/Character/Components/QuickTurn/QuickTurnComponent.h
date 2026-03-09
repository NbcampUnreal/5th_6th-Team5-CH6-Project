// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuickTurnComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WARD_ZERO_API UQuickTurnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuickTurnComponent();

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void StartQuickTurn180();
	void StopQuickTurn();

	bool IsQuickTurning() const { return bIsQuickTurning; }
	int32 GetTurnIndex() const { return TurnIndex; }

private:
	bool bIsQuickTurning = false;
	int32 TurnIndex = 0;
	float TurnAlpha = 0.0f;
	float TurnStartYaw = 0.0f;
	float ControlStartYaw = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Turn")
	float Duration180 = 0.6f;
};

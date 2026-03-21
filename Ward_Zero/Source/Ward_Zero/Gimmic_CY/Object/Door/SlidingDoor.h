// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gimmic_CY/Object/Door/DoorBase.h"
#include "SlidingDoor.generated.h"

UCLASS()
class WARD_ZERO_API ASlidingDoor : public ADoorBase
{
	GENERATED_BODY()
	
public: 
	ASlidingDoor();
	virtual void OpenDoor() override;
	virtual void CloseDoor() override;

protected:
	virtual void BeginPlay() override;

	// Interface
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;

	virtual void UpdateTimelineComp(float Output) override;

	// ��ġ
	FVector ClosedLocation;

	UPROPERTY(EditAnywhere)
	FVector OpenOffset = FVector(120.f, 0.f, 0.f);
};

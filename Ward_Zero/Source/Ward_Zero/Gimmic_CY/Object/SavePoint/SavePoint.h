// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Gimmic_CY/Object/ObjectBase.h"
#include "SavePoint.generated.h"

UCLASS()
class WARD_ZERO_API ASavePoint : public AObjectBase
{
	GENERATED_BODY()

public:
	ASavePoint();
	
	EInteractionType GetInteractionType_Implementation() const override;
	virtual void Activate() override;
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Setting")
	int32 StageIndex;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};

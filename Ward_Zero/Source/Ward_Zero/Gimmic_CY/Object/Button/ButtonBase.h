// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gimmic_CY/Object/ObjectBase.h"
#include "ButtonBase.generated.h"

UCLASS()
class WARD_ZERO_API AButtonBase : public AObjectBase
{
	GENERATED_BODY()

public:
	AButtonBase();
	
	

protected:
	virtual void BeginPlay() override;

public:
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;
	virtual void Activate() override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (DisplayName = "Actors For Activate"), Category="Interaction Actors")
	TArray<AObjectBase*> ActivateActors;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gimmic_CY/Items/ItemBase.h"
#include "DocumentBase.generated.h"

UCLASS()
class WARD_ZERO_API ADocumentBase : public AItemBase
{
	GENERATED_BODY()

public:
	
	ADocumentBase();

protected:
	
	virtual void BeginPlay() override;

	
public:
	
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;
	virtual EInteractionType GetInteractionType_Implementation() const override;
	
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Objects/Interface/Interact.h"
#include "BaseObject.generated.h"

UCLASS()
class WARD_ZERO_API ABaseObject : public AActor , public IInteract
{
	GENERATED_BODY()
	
public:	
	ABaseObject();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:	
	virtual void OnInteract_Implementation(class APrototypeCharacter* Player) override; 

};

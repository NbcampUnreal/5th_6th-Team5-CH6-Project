// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/BaseObject.h"
#include "Ladder.generated.h"

class USceneComponent;
class UBoxComponent;

UCLASS()
class WARD_ZERO_API ALadder : public ABaseObject
{
	GENERATED_BODY()
	
public:
	ALadder();

protected:
	virtual void BeginPlay() override; 
	
public:
	//인터페이스 구현 
	virtual void OnInteract_Implementation(class APrototypeCharacter* Player) override; 
	
public:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> SceneComp;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UBoxComponent> InteractBox;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> BottomStartPoint;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> TopExitPoint;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> LadderMesh;
};

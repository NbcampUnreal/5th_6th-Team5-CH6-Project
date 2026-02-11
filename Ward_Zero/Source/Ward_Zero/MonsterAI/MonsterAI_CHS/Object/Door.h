// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZombieInteractableInterface.h"
#include "GameFramework/Actor.h"
#include "Door.generated.h"

UCLASS()
class WARD_ZERO_API ADoor : public AActor, public IZombieInteractableInterface
{
	GENERATED_BODY()

public:
	
	virtual FOnEntityDestroyed& GetOnDestroyedDelegate() override { return OnDoorDestroyed; }
	virtual float GetEntityHealth() const override { return Health; }
	virtual ANavLinkProxy* GetLinkedNavProxy() const override { return AssociatedProxy; }
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEntityDestroyed OnDoorDestroyed;
	UPROPERTY(EditAnywhere, Category = "Navigation")
	TObjectPtr<ANavLinkProxy> AssociatedProxy;
	
	
	
	protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = 100.0f;
};

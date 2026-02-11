// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/NavLinkProxy.h"
#include "UObject/Interface.h"
#include "ZombieInteractableInterface.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEntityDestroyed);
// This class does not need to be modified.
UINTERFACE()
class UZombieInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class WARD_ZERO_API IZombieInteractableInterface
{
	GENERATED_BODY()

public:
	virtual FOnEntityDestroyed& GetOnDestroyedDelegate() = 0;
	
	virtual float GetEntityHealth() const = 0;
	
	virtual ANavLinkProxy* GetLinkedNavProxy() const = 0;
};

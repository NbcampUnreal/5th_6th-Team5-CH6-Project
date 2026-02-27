// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Navigation/NavLinkProxy.h"
#include "NavLinkProxy_Interaction.generated.h"

UCLASS()
class WARD_ZERO_API ANavLinkProxy_Interaction : public ANavLinkProxy
{
	GENERATED_BODY()

public:
	ANavLinkProxy_Interaction();

	UPROPERTY(EditAnywhere, Category = "Interaction")
	FGameplayTag InteractionTag;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	AActor* InteractedObject = nullptr;
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void HandleSmartLinkReached(AActor* Agent, const FVector& Destination);
};

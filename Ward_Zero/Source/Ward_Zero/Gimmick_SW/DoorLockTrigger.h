// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "DoorLockTrigger.generated.h"

UCLASS()
class WARD_ZERO_API ADoorLockTrigger : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoorLockTrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//UFUNCTION()
	//void OnOverlapBegin(
	//	UPrimitiveComponent* OverlappedComp,
	//	AActor* OtherActor,
	//	UPrimitiveComponent* OtherComp,
	//	int32 OtherBodyIndex,
	//	bool bFromSweep,
	//	const FHitResult& SweepResult
	//);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UBoxComponent* TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lock Area")
	bool bHasBeenTriggered;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lock Area")
	bool bBossKilled;

	void BeginTriggered();
	void EndTriggered();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

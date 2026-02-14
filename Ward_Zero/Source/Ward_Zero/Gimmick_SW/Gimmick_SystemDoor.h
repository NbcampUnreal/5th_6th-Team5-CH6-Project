// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmick_SystemDoor.generated.h"

UCLASS()
class WARD_ZERO_API AGimmick_SystemDoor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGimmick_SystemDoor();

	UFUNCTION(BlueprintCallable, Category = "Door")
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool bHaveCardKey;

	bool bIsSystemDoorOpening; // 문 열림or닫힘 상태 체크

	UPROPERTY(EditAnywhere, Category = "SystemDoor")
	float OpenAngle; // 이동 범위

	UPROPERTY(EditAnywhere, Category = "SystemDoor")
	float RotationSpeed; // 이동 속도

private:
	FRotator StartRotation; // 시작 지점

	FRotator TargetRotation; // 목표 지점

	float CurrentYaw;

	bool bIsRotating;

	bool bIsClosing;

	void OpenSystemDoor();

	void CloseSystemDoor();

	FTimerHandle CloseTimerHandle;

	UPROPERTY(EditAnywhere, Category = "SystemDoor")
	float WaitTimeBeforeClose;
};

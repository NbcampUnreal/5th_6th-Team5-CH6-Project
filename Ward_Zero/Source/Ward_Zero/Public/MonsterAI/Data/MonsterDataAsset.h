// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MonsterDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class WARD_ZERO_API UMonsterDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
	public:
	UPROPERTY(EditAnywhere, Category = "Visual")
	USkeletalMesh* MonsterMesh;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TSubclassOf<UAnimInstance> AnimBPWalkRun; 

	UPROPERTY(EditAnywhere, Category = "Visual")
	FVector MeshScale = FVector(1.0f); 
	
	UPROPERTY(EditAnywhere, Category = "Status")
	float MaxHP = 100.f;
	
	UPROPERTY(EditAnywhere, Category = "Chase")
	float ArrivalRadius = 100.f;
	
	UPROPERTY(EditAnywhere, Category = "Speed")
	float BaseSpeed = 450.f;
	
	UPROPERTY(EditAnywhere, Category = "Speed")
	float ChaseSpeed = 700.f;

	UPROPERTY(EditAnywhere, Category = "Sight")
	float BaseDetectionRange = 1200.f;
	
	UPROPERTY(EditAnywhere, Category = "Sight")
	float LoseSightRange = 1500.f;
	
	UPROPERTY(EditAnywhere, Category = "Sight")
	float ViewAngle = 90.f;
	
	UPROPERTY(EditAnywhere, Category = "Hearing", meta=(ClampMin="0.0", ClampMax="1.0"))
	float HearingThreshold = 0.5f;
	
	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackRange = 150.f;
	
	UPROPERTY(EditAnywhere, Category = "Chase")
	float ChaseRange = 2000.f;
	
	
	UPROPERTY(EditAnywhere, Category = "Sight")
	float EyeHeight = 70.f;
	
	UPROPERTY(EditAnywhere, Category = "Chase")
	float MaxLostTargetTime = 5.f;
	
	
	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* WalkSound;
	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* RunSound;
	
	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* IdleSound;
	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* ChaseSound;
	
	
	UPROPERTY(EditAnywhere, Category = "Volume")
	float IdleSoundVolume = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Volume")
	float ChaseSoundVolume = 1.0f;
	
	UPROPERTY(EditAnywhere, Category = "Duration")
	float StunnedTime = 0.f;
};

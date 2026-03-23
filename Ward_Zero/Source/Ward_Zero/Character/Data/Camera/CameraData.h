// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CameraData.generated.h"

/**
 *
 */
UCLASS()
class WARD_ZERO_API UCameraData : public UDataAsset
{
	GENERATED_BODY()

public:
	// 기본 설정
	UPROPERTY(EditAnywhere, Category = "Base") float DefaultFOV = 60.0f;
	UPROPERTY(EditAnywhere, Category = "Base") float DefaultArmLength = 180.0f;
	UPROPERTY(EditAnywhere, Category = "Base") FVector DefaultSocketOffset = FVector(0.0f, 35.0f, 10.0f);
	UPROPERTY(EditAnywhere, Category = "Base") float InterpSpeed = 15.0f;

	// 조준 설정
	UPROPERTY(EditAnywhere, Category = "Aim") float AimFOV = 50.0f;
	UPROPERTY(EditAnywhere, Category = "Aim") float AimArmLength = 40.0f;
	UPROPERTY(EditAnywhere, Category = "Aim|Pistol") FVector PistolAimSocketOffset = FVector(-20.0f, 30.0f, 20.0f);
	UPROPERTY(EditAnywhere, Category = "Aim|SMG") FVector SMGAimSocketOffset = FVector(-60.0f, 50.0f, 20.0f);
	// 앉기 설정 (여기가 0이면 허리로 갑니다. 35~45 추천)
	UPROPERTY(EditAnywhere, Category = "Crouch") float CrouchedArmLength = 100.f;
	UPROPERTY(EditAnywhere, Category = "Crouch") float CrouchedCameraHeight = 35.f;

	// 흔들림(Bobbing) 설정
	UPROPERTY(EditAnywhere, Category = "Bobbing") float BobFrequency = 12.0f;
	UPROPERTY(EditAnywhere, Category = "Bobbing") float BobAmplitude = 2.0f;
	UPROPERTY(EditAnywhere, Category = "Bobbing") float BobHorizontalAmplitude = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Bobbing") float RunFrequencyAmplify = 1.3f;
	UPROPERTY(EditAnywhere, Category = "Bobbing") float RunAmplitudeAmplify = 1.5f;


	// 숨쉬기/조준 Sway 설정
	UPROPERTY(EditAnywhere, Category = "Sway") float BreathSwayIntensity = 0.6f;
	UPROPERTY(EditAnywhere, Category = "Sway") float WalkSwayIntensity = 0.4f;

	// 베이스 카메라 각도 제한
	UPROPERTY(EditAnywhere, Category = "Limit") float DefaultPitchMin = -60.0f;
	UPROPERTY(EditAnywhere, Category = "Limit") float DefaultPitchMax = 50.0f;

	// 조준 카메라 각도 제한
	UPROPERTY(EditAnywhere, Category = "Limit") float AimPitchMin = -20.0f;
	UPROPERTY(EditAnywhere, Category = "Limit") float AimPitchMax = 10.0f;
};

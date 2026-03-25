#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FlashLightData.generated.h"

UCLASS(BlueprintType)
class WARD_ZERO_API UFlashLightData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // 평상시(걷기/조준) 기본 손전등 수치 
    UPROPERTY(EditAnywhere, Category = "Base Settings")
    float Intensity = 3000.f;    // 밝기 

    UPROPERTY(EditAnywhere, Category = "Base Settings")
    float OuterConeAngle = 35.f; // 원의 크기 

    UPROPERTY(EditAnywhere, Category = "Base Settings", meta = (UIMin = "0.0", UIMax = "1.0"))
    float InnerConeRatio = 0.6f; // 중심부 비율 

    UPROPERTY(EditAnywhere, Category = "Base Settings")
    float AttenuationRadius = 2500.f; // 사거리 

    // 달릴 때 (가슴 부착 시) 보정치 
    UPROPERTY(EditAnywhere, Category = "Sprinting Settings")
    float SprintIntensityMultiplier = 15.0f; // 달리기 밝기 배율 

    UPROPERTY(EditAnywhere, Category = "Sprinting Settings")
    float SprintRadiusMultiplier = 1.5f;     // 달리기 거리 배율 

    // 그래픽 및 그림자 ---
    UPROPERTY(EditAnywhere, Category = "Visual Settings") // 맵에서 포그 사용 시 빛의 선명함 여부 
    float VolumetricScatteringIntensity = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Visual Settings")
    bool bCastShadows = false; // 손전등 빛이 물체에 닿을 때 그림자 여부 

    // 벽에 다가갈 때 빛이 모이는 효과 
    UPROPERTY(EditAnywhere, Category = "Focus Settings")
    bool bEnableDynamicFocus = true; 

    UPROPERTY(EditAnywhere, Category = "Focus Settings")
    float MaxFocusDistance = 500.f; // 초점 시작 거리 

    UPROPERTY(EditAnywhere, Category = "Focus Settings")
    float MinOuterConeAngle = 12.f; // 근접 눈부심 배율 

    UPROPERTY(EditAnywhere, Category = "Focus Settings")
    float CloseRangeIntensityMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, Category = "Sound")
    USoundBase* OnSound;


    UPROPERTY(EditAnywhere, Category = "Sound")
    USoundBase* OffSound;
};
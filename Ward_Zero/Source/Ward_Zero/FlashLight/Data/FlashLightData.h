#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FlashLightData.generated.h"

UCLASS(BlueprintType)
class WARD_ZERO_API UFlashLightData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // --- [Base Settings] 평상시/조준 시 기본 수치 ---
    UPROPERTY(EditAnywhere, Category = "Base Settings")
    float Intensity = 3000.f;

    UPROPERTY(EditAnywhere, Category = "Base Settings")
    float OuterConeAngle = 35.f;

    // Outer 대비 Inner 비율 (0.5면 절반이 쨍함)
    UPROPERTY(EditAnywhere, Category = "Base Settings", meta = (UIMin = "0.0", UIMax = "1.0"))
    float InnerConeRatio = 0.6f;

    UPROPERTY(EditAnywhere, Category = "Base Settings")
    float AttenuationRadius = 2500.f;

    // --- [Sprinting Settings] 달릴 때(가슴 라이트) 보정치 ---
    // 가슴 소켓은 뒤에 있으므로 밝기 배율이 필요합니다 (기본 10.0 이상 권장)
    UPROPERTY(EditAnywhere, Category = "Sprinting Settings")
    float SprintIntensityMultiplier = 15.0f;

    UPROPERTY(EditAnywhere, Category = "Sprinting Settings")
    float SprintRadiusMultiplier = 1.5f;

    // --- [Ambient Settings] 손전등이 꺼져 있을 때 최소 시야 ---
    UPROPERTY(EditAnywhere, Category = "Ambient Settings")
    float AmbientIntensity = 1000.f;

    UPROPERTY(EditAnywhere, Category = "Ambient Settings")
    float AmbientRadius = 500.f;

    // --- [Visual & Shadow Settings] ---
    UPROPERTY(EditAnywhere, Category = "Visual Settings")
    float VolumetricScatteringIntensity = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Visual Settings")
    bool bCastShadows = false;

    UPROPERTY(EditAnywhere, Category = "Material Settings")
    float EmissiveIntensity = 50.f;

    // --- [Dynamic Focus Settings] ---
    UPROPERTY(EditAnywhere, Category = "Focus Settings")
    bool bEnableDynamicFocus = true;

    // 이 거리(cm)보다 가까워지면 초점을 맞추기 시작합니다.
    UPROPERTY(EditAnywhere, Category = "Focus Settings")
    float MaxFocusDistance = 500.f;

    // 벽에 완전히 붙었을 때의 최소 각도
    UPROPERTY(EditAnywhere, Category = "Focus Settings")
    float MinOuterConeAngle = 12.f;

    // 가까워질 때 밝기 보정 배율 (가까울수록 더 눈부시게)
    UPROPERTY(EditAnywhere, Category = "Focus Settings")
    float CloseRangeIntensityMultiplier = 1.5f;
};

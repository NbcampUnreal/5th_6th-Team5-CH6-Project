#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FlashLightData.generated.h"

UCLASS(BlueprintType)
class WARD_ZERO_API UFlashLightData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Light Settings")
    float Intensity = 0.f;

    UPROPERTY(EditAnywhere, Category = "Light Settings")
    float OuterConeAngle = 0.f;

    UPROPERTY(EditAnywhere, Category = "Light Settings")
    float AttenuationRadius = 0.f;

    UPROPERTY(EditAnywhere, Category = "Material Settings")
    float EmissiveIntensity = 0.f;
};

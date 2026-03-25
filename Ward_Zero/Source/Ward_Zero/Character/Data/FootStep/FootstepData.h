#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FootstepData.generated.h"

USTRUCT(BlueprintType)
struct FFootstepInfo
{
    GENERATED_BODY()

    FFootstepInfo()
        : StepSound(nullptr)
        , NoiseLoudness(0.4f)
        , NoiseRange(1000.0f)
    {}

    UPROPERTY(EditAnywhere) USoundBase* StepSound;
    UPROPERTY(EditAnywhere) float NoiseLoudness; 
    UPROPERTY(EditAnywhere) float NoiseRange;
};

UCLASS()
class WARD_ZERO_API UFootstepData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    // Surface 타입별로 데이터 매핑
    UPROPERTY(EditAnywhere, Category = "Footstep")
    TMap<TEnumAsByte<EPhysicalSurface>, FFootstepInfo> SurfaceFootstepMap;
};

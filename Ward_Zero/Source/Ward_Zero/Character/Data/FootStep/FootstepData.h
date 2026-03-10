#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FootstepData.generated.h"

USTRUCT(BlueprintType)
struct FFootstepInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) USoundBase* StepSound;
    UPROPERTY(EditAnywhere) float NoiseLoudness = 0.4f;
    UPROPERTY(EditAnywhere) float NoiseRange = 1000.0f;
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

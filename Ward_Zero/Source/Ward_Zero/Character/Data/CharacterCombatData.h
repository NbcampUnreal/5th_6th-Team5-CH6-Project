#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterCombatData.generated.h"

UCLASS(BlueprintType)
class WARD_ZERO_API UCharacterCombatData : public UPrimaryDataAsset
{
	GENERATED_BODY()
    
protected:
    virtual FPrimaryAssetId GetPrimaryAssetId() const override;

public:
    UPROPERTY(EditAnywhere, Category = "Recoil")
    float RecoilInterpSpeed = 25.f;
    UPROPERTY(EditAnywhere, Category = "Recoil")
    float RecoilRecoverySpeed = 5.f;

    UPROPERTY(EditAnywhere, Category = "Spread")
    float MaxSpread = 5.f;
    UPROPERTY(EditAnywhere, Category = "Spread")
    float FireSpreadPenalty = 2.5f;
};

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterStatusData.generated.h"

UCLASS(BlueprintType)
class WARD_ZERO_API UCharacterStatusData : public UPrimaryDataAsset
{
    GENERATED_BODY()

protected:
    virtual FPrimaryAssetId GetPrimaryAssetId() const override;

public:
    UPROPERTY(EditAnywhere, Category = "Health")
    float MaxHealth = 100.f;

    UPROPERTY(EditAnywhere, Category = "Stamina")
    float MaxStamina = 100.f;

    UPROPERTY(EditAnywhere, Category = "Stamina")
    float StaminaDrainRate = 10.f;

    UPROPERTY(EditAnywhere, Category = "Heath|Heal")
    float HealAmount = 30.0f;

    UPROPERTY(EditAnywhere, Category = "Sound")
    TObjectPtr<USoundBase> HitSound;

    UPROPERTY(EditAnywhere, Category = "Sound")
    TObjectPtr<USoundBase> DeathSound;
};

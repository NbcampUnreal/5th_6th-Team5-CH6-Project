#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterMovementData.generated.h"


UCLASS(BlueprintType)
class WARD_ZERO_API UCharacterMovementData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    virtual FPrimaryAssetId GetPrimaryAssetId() const override { return FPrimaryAssetId("CharacterData", GetFName()); }

    UPROPERTY(EditAnywhere, Category = "Speed")
    float WalkSpeed = 200.f;

    UPROPERTY(EditAnywhere, Category = "Speed")
    float RunSpeed = 450.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float CrouchMovementSpeed = 150.f;

    UPROPERTY(EditAnywhere, Category = "Camera")
    float DefaultArmLength = 180.f;

    UPROPERTY(EditAnywhere, Category = "Camera")
    float AimArmLength = 40.f;

    UPROPERTY(EditAnywhere, Category = "Camera|Bobbing")
    float BobFrequency = 12.0f;

    UPROPERTY(EditAnywhere, Category = "Camera|Bobbing")
    float BobAmplitude = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Camera|Offset")
    FVector PistolAimSocketOffset = FVector(-20.0f, 30.0f, 20.0f);
};

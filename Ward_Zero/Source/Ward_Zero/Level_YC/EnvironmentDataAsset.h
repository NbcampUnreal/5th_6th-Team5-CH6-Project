#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnvironmentDataAsset.generated.h" 

USTRUCT(BlueprintType)
struct FEnvironmentSettings
{
    GENERATED_BODY()

    // Sun 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor::White;

    // RectLight 설정 (태그 ReactLight 용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RectLight")
    float RectIntensity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RectLight")
    FLinearColor RectColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RectLight")
    float RectSourceWidth = 64.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RectLight")
    float RectSourceHeight = 64.0f;

    // Fog 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f);

    // PostProcess 설정 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float BloomIntensity = 0.675f;
};

UCLASS(BlueprintType)
class WARD_ZERO_API UEnvironmentDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FEnvironmentSettings Settings;
};
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnvironmentDataAsset.h"
#include "EnvManager.generated.h"

class ADirectionalLight;
class AExponentialHeightFog;
class APostProcessVolume;
class ARectLight; 

UCLASS()
class WARD_ZERO_API AEnvManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvManager();

protected:
    virtual void BeginPlay() override;

public:
    // 적용할 데이터 에셋
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    UEnvironmentDataAsset* ActiveEnvAsset;

    // 컨트롤 대상 액터들 (단일)
    UPROPERTY(EditAnywhere, Category = "Environment|Targets")
    ADirectionalLight* TargetSun;

    UPROPERTY(EditAnywhere, Category = "Environment|Targets")
    AExponentialHeightFog* TargetFog;

    UPROPERTY(EditAnywhere, Category = "Environment|Targets")
    APostProcessVolume* TargetPostProcess;

    // 컨트롤 대상 (태그 기반 RectLight 배열)
    UPROPERTY(EditAnywhere, Category = "Environment|Targets")
    TArray<ARectLight*> TargetRectLights;

    
    // 태그 "RectLight"를 가진 액터를 모두 찾아 배열에 추가
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Environment")
    void FindRectLightsByTag();

    // 데이터 에셋의 수치를 모든 타겟에 적용
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Environment")
    void ApplyEnvironment();
};
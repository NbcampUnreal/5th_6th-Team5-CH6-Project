#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnvManager.generated.h"

class USoundBase;
class UAudioComponent;
class ALight;
class APostProcessVolume;

UENUM(BlueprintType)
enum class EEnvZone : uint8
{
    B1F, F1, F2, BossRoom, None
};

USTRUCT(BlueprintType)
struct FZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Zone")
    EEnvZone ZoneType = EEnvZone::None;

    UPROPERTY(EditAnywhere, Category = "Zone")
    TArray<ALight*> ZoneLights;

    UPROPERTY(EditAnywhere, Category = "Zone")
    APostProcessVolume* ZonePostProcess;
};

UCLASS()
class WARD_ZERO_API AEnvManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvManager();

protected:
    virtual void BeginPlay() override;

public:
    
    UPROPERTY(EditAnywhere, Category = "Environment")
    TArray<FZoneConfig> ZoneConfigs;

    
    UPROPERTY(EditAnywhere, Category = "Environment|Sound")
    USoundBase* BaseNormalBGM; 

    UPROPERTY(EditAnywhere, Category = "Environment|Sound")
    USoundBase* HutonBGM;

    UPROPERTY(EditAnywhere, Category = "Environment|Sound")
    USoundBase* TentacleBGM;

    
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SwitchZone(EEnvZone NewZone);

    // AI 호출용 BGM 함수들 
    UFUNCTION(BlueprintCallable, Category = "Environment|Sound")
    void PlayHutonBGM();

    UFUNCTION(BlueprintCallable, Category = "Environment|Sound")
    void PlayTentacleBGM();

    UFUNCTION(BlueprintCallable, Category = "Environment|Sound")
    void RestoreNormalBGM();

private:
    UPROPERTY(VisibleAnywhere, Category = "Environment")
    EEnvZone CurrentZone = EEnvZone::None;

    UPROPERTY(VisibleAnywhere, Category = "Environment")
    UAudioComponent* BGMComponent;

    FZoneConfig* GetConfig(EEnvZone Zone);
    void PlayFadeMusic(USoundBase* NewMusic);
};
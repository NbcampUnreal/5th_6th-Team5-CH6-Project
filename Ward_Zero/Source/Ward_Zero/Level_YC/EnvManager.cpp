#include "Level_YC/EnvManager.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/RectLight.h"          
#include "Components/RectLightComponent.h" 
#include "Kismet/GameplayStatics.h"   

AEnvManager::AEnvManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AEnvManager::BeginPlay()
{
    Super::BeginPlay();
    ApplyEnvironment();
}

void AEnvManager::FindRectLightsByTag()
{
    TargetRectLights.Empty();

    TArray<AActor*> FoundActors;
    // 월드 내의 모든 RectLight를 검색
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARectLight::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        // 태그가 "RectLight"인 경우만 리스트에 추가
        if (Actor && Actor->ActorHasTag(FName("RectLight")))
        {
            TargetRectLights.Add(Cast<ARectLight>(Actor));
        }
    }
} 

void AEnvManager::ApplyEnvironment()
{
    if (!ActiveEnvAsset) return;

    const FEnvironmentSettings& Settings = ActiveEnvAsset->Settings;

    // 1. 태양광 설정
    if (TargetSun && TargetSun->GetLightComponent())
    {
        TargetSun->GetLightComponent()->SetIntensity(Settings.SunIntensity);
        TargetSun->GetLightComponent()->SetLightColor(Settings.SunColor);
    }

    // 2. 안개 설정
    if (TargetFog && TargetFog->GetComponent())
    {
        TargetFog->GetComponent()->SetFogDensity(Settings.FogDensity);
        TargetFog->GetComponent()->SetFogInscatteringColor(Settings.FogColor);
    }

    // 3. 포스트 프로세스 설정
    if (TargetPostProcess)
    {
        TargetPostProcess->Settings.bOverride_BloomIntensity = true;
        TargetPostProcess->Settings.BloomIntensity = Settings.BloomIntensity;
    }

    // 4. RectLight 설정 (배열 순회)
    for (ARectLight* RectLight : TargetRectLights)
    {
        if (RectLight && RectLight->GetLightComponent())
        {
            URectLightComponent* RectComp = Cast<URectLightComponent>(RectLight->GetLightComponent());
            if (RectComp)
            {
                RectComp->SetIntensity(Settings.RectIntensity);
                RectComp->SetLightColor(Settings.RectColor);
                RectComp->SetSourceWidth(Settings.RectSourceWidth);
                RectComp->SetSourceHeight(Settings.RectSourceHeight);
            }
        }
    }
} 
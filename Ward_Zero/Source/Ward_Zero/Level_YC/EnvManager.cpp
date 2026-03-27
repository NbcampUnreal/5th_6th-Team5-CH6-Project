#include "EnvManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/Light.h"
#include "Components/LightComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/GameplayStatics.h"

AEnvManager::AEnvManager()
{
    PrimaryActorTick.bCanEverTick = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    BGMComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BGMComponent"));
    BGMComponent->bAutoActivate = false;
    BGMComponent->SetupAttachment(RootComponent);
}

void AEnvManager::BeginPlay()
{
    Super::BeginPlay(); 

    for (FZoneConfig& Config : ZoneConfigs)
    {
        SetZoneState(Config, false);
    }

    if (BaseNormalBGM)
    {
        PlayFadeMusic(BaseNormalBGM);
    }

    CurrentZone = EEnvZone::None;
    SwitchZone(EEnvZone::B1F);
}


void AEnvManager::SetZoneState(FZoneConfig& Config, bool bActive)
{
    
    if (Config.ZonePostProcess)
    {
        Config.ZonePostProcess->bEnabled = bActive;
        if (bActive)
        {
            Config.ZonePostProcess->Priority = 10.0f;
        }
    }

    for (ALight* Light : Config.ZoneLights)
    {
        if (Light && Light->GetLightComponent())
        {
            ULightComponent* LightComp = Light->GetLightComponent();

            LightComp->SetVisibility(bActive);
            LightComp->bAffectsWorld = bActive;
            LightComp->SetCastShadows(bActive);
            LightComp->MarkRenderStateDirty();
        }
    }
}

void AEnvManager::SwitchZone(EEnvZone NewZone)
{
    if (CurrentZone == NewZone || NewZone == EEnvZone::None) return;

    if (FZoneConfig* CurrentConfig = GetConfig(CurrentZone))
    {
        SetZoneState(*CurrentConfig, false);
    }

    CurrentZone = NewZone;
    if (FZoneConfig* NewConfig = GetConfig(CurrentZone))
    {
        SetZoneState(*NewConfig, true);
    }
}

FZoneConfig* AEnvManager::GetConfig(EEnvZone Zone)
{
    for (FZoneConfig& Config : ZoneConfigs)
    {
        if (Config.ZoneType == Zone)
        {
            return &Config;
        }
    }
    return nullptr;
}

void AEnvManager::PlayFadeMusic(USoundBase* NewMusic)
{
    if (!NewMusic || !BGMComponent) return;

    if (BGMComponent->IsPlaying() && BGMComponent->GetSound() == NewMusic) return;

    BGMComponent->bAllowSpatialization = false;
    BGMComponent->SetUISound(true);

    BGMComponent->FadeOut(1.5f, 0.0f);
    BGMComponent->SetSound(NewMusic);
    BGMComponent->FadeIn(1.5f, 1.0f, 0.0f);
}

void AEnvManager::PlayHutonBGM() { PlayFadeMusic(HutonBGM); }
void AEnvManager::PlayHutonPhase2BGM() { PlayFadeMusic(HutonPhase2BGM); }
void AEnvManager::PlayTentacleBGM() { PlayFadeMusic(TentacleBGM); }
void AEnvManager::RestoreNormalBGM() { PlayFadeMusic(BaseNormalBGM); }
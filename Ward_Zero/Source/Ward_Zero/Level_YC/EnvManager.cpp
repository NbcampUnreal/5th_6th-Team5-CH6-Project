#include "EnvManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/Light.h"
#include "Components/LightComponent.h"
#include "Engine/PostProcessVolume.h"

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
        if (Config.ZonePostProcess)
        {
            Config.ZonePostProcess->bEnabled = false;
        }

        for (ALight* Light : Config.ZoneLights)
        {
            if (Light && Light->GetLightComponent())
            {
                ULightComponent* LightComp = Light->GetLightComponent();

               
                LightComp->bAffectsWorld = false;
                LightComp->SetCastShadows(false);
                LightComp->SetVisibility(false);

                
                LightComp->MarkRenderStateDirty();
            }
        }
    }

    
    if (BaseNormalBGM) PlayFadeMusic(BaseNormalBGM);

    
    SwitchZone(EEnvZone::B1F);
}

void AEnvManager::SwitchZone(EEnvZone NewZone)
{
    if (CurrentZone == NewZone) return;

    
    if (FZoneConfig* OldConfig = GetConfig(CurrentZone))
    {
        if (OldConfig->ZonePostProcess) OldConfig->ZonePostProcess->bEnabled = false;

        for (ALight* Light : OldConfig->ZoneLights)
        {
            if (Light && Light->GetLightComponent())
            {
                ULightComponent* LightComp = Light->GetLightComponent();
                LightComp->bAffectsWorld = false;
                LightComp->SetCastShadows(false);
                LightComp->SetVisibility(false);
                LightComp->MarkRenderStateDirty();
            }
        }
    }

    
    CurrentZone = NewZone;

    
    if (FZoneConfig* NewConfig = GetConfig(CurrentZone))
    {
        if (NewConfig->ZonePostProcess)
        {
            NewConfig->ZonePostProcess->bEnabled = true;
            NewConfig->ZonePostProcess->Priority = 10.0f;
        }

        for (ALight* Light : NewConfig->ZoneLights)
        {
            if (Light && Light->GetLightComponent())
            {
                ULightComponent* LightComp = Light->GetLightComponent();
                LightComp->bAffectsWorld = true;
                LightComp->SetCastShadows(true);
                LightComp->SetVisibility(true);
                LightComp->MarkRenderStateDirty();
            }
        }
    }
}

void AEnvManager::PlayHutonBGM() { PlayFadeMusic(HutonBGM); }
void AEnvManager::PlayTentacleBGM() { PlayFadeMusic(TentacleBGM); }
void AEnvManager::RestoreNormalBGM() { PlayFadeMusic(BaseNormalBGM); }

FZoneConfig* AEnvManager::GetConfig(EEnvZone Zone)
{
    for (FZoneConfig& Config : ZoneConfigs)
    {
        if (Config.ZoneType == Zone) return &Config;
    }
    return nullptr;
}

void AEnvManager::PlayFadeMusic(USoundBase* NewMusic)
{
    if (!NewMusic || !BGMComponent) return;
    if (BGMComponent->IsPlaying() && BGMComponent->GetSound() == NewMusic) return;

    BGMComponent->FadeOut(1.5f, 0.0f);
    BGMComponent->SetSound(NewMusic);
    BGMComponent->FadeIn(1.5f, 1.0f, 0.0f);
}
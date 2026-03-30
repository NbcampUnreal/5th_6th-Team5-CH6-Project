// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseZombieActivateTrigger.h"

#include "WardGameInstanceSubsystem.h"
#include "MonsterAI/MonsterAI_CHS/Entity/BaseZombie.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"


ABaseZombieActivateTrigger::ABaseZombieActivateTrigger()
{
	PrimaryActorTick.bCanEverTick = false;
	
	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	RootComponent = TriggerVolume;
	
	TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
	bHasTriggered = false;
}

void ABaseZombieActivateTrigger::WakeUpZombies()
{
	if (ZomIdx >= ZombieNum)
	{
		GetWorldTimerManager().ClearTimer(WaveTimerHandle);
		return;
	}
	int StartIdx = ZomIdx;
	int EndIdx = FMath::Clamp(ZomIdx + ZombieNumPerOneWave,ZomIdx,ZombieNum);
	for (int i = StartIdx; i < EndIdx; ++i)
	{
		ABaseZombie* Zombie = Zombies[i];
		if (Zombie)
		{
			Zombie->Activate();
		}
	}
	ZomIdx = EndIdx;
}

void ABaseZombieActivateTrigger::BeginPlay()
{
	Super::BeginPlay();
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UWardGameInstanceSubsystem* WGI =  GI->GetSubsystem<UWardGameInstanceSubsystem>())
		{
			if (StageIndex < WGI->GetCurrentStage()) return;
		}
	}
	
	
	if (TriggerVolume)
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this,&ABaseZombieActivateTrigger::OnTriggerOverlap);
	}
	ZombieNum = Zombies.Num();
}

void ABaseZombieActivateTrigger::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bHasTriggered) return;
	
	if (OtherActor && OtherActor != this && OtherActor->ActorHasTag(TEXT("Player")))
	{
		ActivateTrigger();
	}
}

void ABaseZombieActivateTrigger::ActivateTrigger()
{
	bHasTriggered = true;
	WakeUpZombies();
	GetWorldTimerManager().SetTimer(
		WaveTimerHandle, 
		this, 
		&ABaseZombieActivateTrigger::WakeUpZombies, 
		WaveInterval, 
	true 
	);
		
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}



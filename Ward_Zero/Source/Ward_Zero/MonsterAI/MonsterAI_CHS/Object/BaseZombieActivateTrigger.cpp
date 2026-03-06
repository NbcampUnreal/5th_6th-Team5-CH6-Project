// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseZombieActivateTrigger.h"
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

void ABaseZombieActivateTrigger::BeginPlay()
{
	Super::BeginPlay();
	if (TriggerVolume)
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this,&ABaseZombieActivateTrigger::OnTriggerOverlap);
	}
}

void ABaseZombieActivateTrigger::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bHasTriggered) return;
	
	if (OtherActor && OtherActor != this && OtherActor->ActorHasTag(TEXT("Player")))
	{
		bHasTriggered = true;
		
		for (ABaseZombie* Zombie: Zombies)
		{
			if (Zombie)
			{
				Zombie->Activate();
			}
		}
		TriggerVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}



// Fill out your copyright notice in the Description page of Project Settings.


#include "SavePoint.h"

#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "UI_KWJ/Save/SaveSubsystem.h"


// Sets default values
ASavePoint::ASavePoint()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

EInteractionType ASavePoint::GetInteractionType_Implementation() const
{
	//return EInteractionType::Save;
	return EInteractionType::Save;
}

void ASavePoint::Activate()
{
	Super::Activate();
}

void ASavePoint::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (!Character)
	{
		return;
	}
	if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
	{
		if (ULocalPlayer* LP = Cast<ULocalPlayer>(PC->GetLocalPlayer()))
		{
			if (USaveSubsystem* SaveSubsys = LP->GetSubsystem<USaveSubsystem>())
			{
				SaveSubsys->ShowSaveUI();
			}
		}
	}
	
}


// Called when the game starts or when spawned
void ASavePoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASavePoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


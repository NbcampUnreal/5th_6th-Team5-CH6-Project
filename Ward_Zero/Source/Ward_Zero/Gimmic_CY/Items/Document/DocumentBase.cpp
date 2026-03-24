// Fill out your copyright notice in the Description page of Project Settings.


#include "DocumentBase.h"
#include "WardGameInstanceSubsystem.h"
#include "UI_KWJ/Reading/DocumentSubsystem.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"


// Sets default values
ADocumentBase::ADocumentBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ADocumentBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADocumentBase::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (!Character || !bCanInteract) return;
	Super::HandleInteraction_Implementation(Character);
	
	UWardGameInstanceSubsystem* SaveGI = GetGameInstance()->GetSubsystem<UWardGameInstanceSubsystem>();
	if (SaveGI)
	{
		SaveGI->ActivateDocumentIndex(DocIdx);
	}

	// 서류 뷰어 열기
	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (PC)
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UDocumentSubsystem* DocSys = LP->GetSubsystem<UDocumentSubsystem>())
			{
				DocSys->OpenDocumentByIndex(DocIdx);
			}
		}
	}
}

EInteractionType ADocumentBase::GetInteractionType_Implementation() const
{
	return EInteractionType::Heal;
}



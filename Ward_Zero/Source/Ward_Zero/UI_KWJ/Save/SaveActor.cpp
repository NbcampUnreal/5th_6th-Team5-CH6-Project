// SaveActor.cpp

#include "UI_KWJ/Save/SaveActor.h"
#include "UI_KWJ/Save/SaveSubsystem.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Ward_Zero.h"

ASaveActor::ASaveActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SaveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SaveMesh"));
	RootComponent = SaveMesh;
}

EInteractionType ASaveActor::GetInteractionType_Implementation() const
{
	return EInteractionType::Save;
}

bool ASaveActor::SetBCanInteract(bool IsCanInteract)
{
	return false;
}

bool ASaveActor::GetBCanInteract() const
{
	return false;
}

void ASaveActor::OnIneracted_Implementation(APrototypeCharacter* Character)
{
	if (!Character) return;

	UE_LOG(LogWard_Zero, Log, TEXT("세이브 포인트 상호작용"));

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	USaveSubsystem* SaveSub = LP->GetSubsystem<USaveSubsystem>();
	if (SaveSub)
	{
		SaveSub->ShowSaveUI();
	}
}

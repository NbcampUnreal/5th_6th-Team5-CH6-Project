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

void ASaveActor::OnInteract_Implementation(APrototypeCharacter* Player)
{
	if (!Player) return;

	UE_LOG(LogWard_Zero, Log, TEXT("세이브 포인트 상호작용"));

	APlayerController* PC = Cast<APlayerController>(Player->GetController());
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	USaveSubsystem* SaveSub = LP->GetSubsystem<USaveSubsystem>();
	if (SaveSub)
	{
		SaveSub->ShowSaveUI();
	}
}

// DocumentPickupActor.cpp

#include "UI_KWJ/Reading/DocumentPickupActor.h"
#include "UI_KWJ/Reading/DocumentData.h"
#include "UI_KWJ/Reading/DocumentSubsystem.h"
#include "../Character/Prototype_Character/PrototypeCharacter.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Ward_Zero.h"

ADocumentPickupActor::ADocumentPickupActor()
{
	PrimaryActorTick.bCanEverTick = false;

	DocumentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DocumentMesh"));
	DocumentMesh->SetupAttachment(RootComponent);
}

void ADocumentPickupActor::BeginPlay()
{
	Super::BeginPlay();
}

void ADocumentPickupActor::OnInteract_Implementation(APrototypeCharacter* Player)
{
	if (!bCanInteract || !DocumentData || !Player) return;

	APlayerController* PC = Cast<APlayerController>(Player->GetController());
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	UE_LOG(LogWard_Zero, Log, TEXT("서류 열기: %s"), *DocumentData->DocumentTitle.ToString());

	UDocumentSubsystem* DocSubsystem = LP->GetSubsystem<UDocumentSubsystem>();
	if (DocSubsystem)
	{
		DocSubsystem->OpenDocument(DocumentData);
	}
}

// DocumentPickupActor.cpp

#include "UI_KWJ/Reading/DocumentPickupActor.h"
#include "UI_KWJ/Reading/DocumentData.h"
#include "UI_KWJ/Reading/DocumentSubsystem.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Ward_Zero.h"

ADocumentPickupActor::ADocumentPickupActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 콜리전 전용 박스 (라인트레이스에 잡히는 역할)
	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	RootComponent = InteractionBox;
	InteractionBox->SetBoxExtent(FVector(30.0f, 30.0f, 30.0f));
	InteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// 비주얼 메시 (콜리전 없음, 보이기만 함)
	DocumentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DocumentMesh"));
	DocumentMesh->SetupAttachment(InteractionBox);
	DocumentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ADocumentPickupActor::BeginPlay()
{
	Super::BeginPlay();
}

EInteractionType ADocumentPickupActor::GetInteractionType_Implementation() const
{
	return EInteractionType::Document;
}

bool ADocumentPickupActor::SetBCanInteract(bool IsCanInteract)
{
	bCanInteract = IsCanInteract;
	return bCanInteract;
}

bool ADocumentPickupActor::GetBCanInteract() const
{
	return bCanInteract;
}

//void ADocumentPickupActor::HiddenActor()
//{
//}

bool ADocumentPickupActor::CanBeInteracted_Implementation() const
{
	return bCanInteract && DocumentData != nullptr;
}

void ADocumentPickupActor::OnIneracted_Implementation(APrototypeCharacter* Character)
{
	if (!bCanInteract || !DocumentData || !Character) return;

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
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

#include "Gimmic_CY/AKeyCard.h"
#include "Components/StaticMeshComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Components/BoxComponent.h"

// Sets default values
AAKeyCard::AAKeyCard()
{
	PrimaryActorTick.bCanEverTick = false;

	KeyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeyMesh"));
	RootComponent = KeyMesh;

	KeyMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	KeyMesh->SetCollisionResponseToAllChannels(ECR_Block);
	KeyMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetupAttachment(KeyMesh);
}

void AAKeyCard::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAKeyCard::OnIneracted_Implementation(APrototypeCharacter* Character)
{
	if (IInteractionBase::Execute_CanBeInteracted(this))
	{
		IInteractionBase::Execute_HandleInteraction(this, Character);
	}
}

void AAKeyCard::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (!Character) return;

	// 플레이어에게 카드키 지급
	Character->GiveKeyCard();
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, "Key Card Acquired");

	// 카드키 제거
	Destroy();
}

EInteractionType AAKeyCard::GetInteractionType_Implementation() const
{
	return EInteractionType::Key;
}


#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Gimmic_CY/InteractionBase.h"
#include "DoorActor.h"
#include "Gimmic_CY/Lever.h"

ALever::ALever()
{
	PrimaryActorTick.bCanEverTick = false;

	// Scene Root
	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	RootComponent = InteractionBox;

	Lever = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Lever"));
	Lever->SetupAttachment(InteractionBox);
	Lever->SetCollisionResponseToChannels(ECR_Block);

}

void ALever::BeginPlay()
{
	Super::BeginPlay();
}

void ALever::LeverOpenDoor()
{
	for (ADoorActor* doorActor : DoorsForOpen)
	{
		if (doorActor)
		{
			doorActor->OpenDoor();
		}
	}
}

void ALever::LeverCloseDoor()
{
	for (ADoorActor* doorColseActor : DoorsForClose)
	{
		if (doorColseActor)
		{
			doorColseActor->CloseDoor();
		}
	}
}

void ALever::LeverLockInteraction()
{
	for (AActor* Objects : InteractionActors)
	{
		IInteractionBase* interactionActorBase = Cast<IInteractionBase>(Objects);
		if (interactionActorBase)
		{
			interactionActorBase->SetBCanInteract(false);
		}
	}
}

void ALever::LeverUnLockInteraction()
{
	for (AActor* Objects : UnInteractionActors)
	{
		IInteractionBase* interactionActorBase = Cast<IInteractionBase>(Objects);
		if (interactionActorBase)
		{
			interactionActorBase->SetBCanInteract(true);
		}
	}
}

void ALever::OnIneractionRangeEntered_Implementation()
{
}

void ALever::OnIneractionRangeExited_Implementation()
{
}

void ALever::OnIneracted_Implementation(APrototypeCharacter* Character)
{
}

void ALever::HandleInteraction_Implementation(APrototypeCharacter* Character)
{

}

EInteractionType ALever::GetInteractionType_Implementation() const
{
	return EInteractionType();
}

bool ALever::SetBCanInteract(bool IsCanInteract)
{
	return false;
}

bool ALever::GetBCanInteract() const
{
	return false;
}



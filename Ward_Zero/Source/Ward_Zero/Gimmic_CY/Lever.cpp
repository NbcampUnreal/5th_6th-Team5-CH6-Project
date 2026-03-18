#include "Gimmic_CY/Lever.h"
#include "Gimmic_CY/Base/InteractionBase.h"
#include "Gimmic_CY/Door/SingleDoor.h"

ALever::ALever()
{
}

void ALever::LeverOpenDoor()
{
	for (ASingleDoor* doorActor : DoorsForOpen)
	{
		if (doorActor)
		{
			doorActor->OpenDoor();
		}
	}
}

void ALever::LeverCloseDoor()
{
	for (ASingleDoor* doorActor : DoorsForOpen)
	{
		if (doorActor)
		{
			doorActor->CloseDoor();
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

void ALever::ActivateLever()
{
	LeverOpenDoor();
	LeverUnLockInteraction();
	LeverCloseDoor();
	LeverLockInteraction();

	bCanInteract = false;
}

void ALever::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (bCanInteract)
	{
		LeverOpenDoor();
		LeverLockInteraction();
		LeverCloseDoor();
		LeverUnLockInteraction();
	}

	bCanInteract = false;
}

#include "Gimmic_CY/Object/Door/SlidingDoor.h"

ASlidingDoor::ASlidingDoor()
{

}

void ASlidingDoor::OpenDoor()
{
	Super::OpenDoor();
	DoorTimelineComp->Play();
	bIsOpen = true;
}

void ASlidingDoor::CloseDoor()
{
	Super::CloseDoor();
	DoorTimelineComp->Reverse();
}

void ASlidingDoor::BeginPlay()
{
	Super::BeginPlay();

	ClosedLocation = Mesh->GetRelativeLocation();
	
}

void ASlidingDoor::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (!bCanInteract)
		return;
	if (!DoorTimelineFloatCurve)
		return;

	if (!bIsOpen)
	{
		Activate();
	}
}

void ASlidingDoor::UpdateTimelineComp(float Output)
{
	FVector OpenLocation = ClosedLocation + OpenOffset;

	FVector NewLocation = FMath::Lerp(ClosedLocation, OpenLocation, Output);

	Mesh->SetRelativeLocation(NewLocation);
}


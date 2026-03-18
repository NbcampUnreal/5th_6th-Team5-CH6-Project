#include "Gimmic_CY/Door/SlidingDoor.h"

ASlidingDoor::ASlidingDoor()
{

}

void ASlidingDoor::BeginPlay()
{
	Super::BeginPlay();

	ClosedLocation = Mesh->GetRelativeLocation();
	UpdateFunctionFloat.BindDynamic(this, &ASlidingDoor::UpdateTimelineComp);
	if (DoorTimelineFloatCurve)
	{
		DoorTimelineComp->AddInterpFloat(DoorTimelineFloatCurve, UpdateFunctionFloat);
	}
}

void ASlidingDoor::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (!DoorTimelineFloatCurve)
		return;

	if (!bIsOpen)
	{
		DoorTimelineComp->Play();
	}
	else
	{
		DoorTimelineComp->Reverse();
	}

	bIsOpen = !bIsOpen;
}

void ASlidingDoor::UpdateTimelineComp(float Output)
{
	FVector OpenLocation = ClosedLocation + OpenOffset;

	FVector NewLocation = FMath::Lerp(ClosedLocation, OpenLocation, Output);

	Mesh->SetRelativeLocation(NewLocation);
}


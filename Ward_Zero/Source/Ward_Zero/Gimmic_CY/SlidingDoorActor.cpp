#include "Gimmic_CY/SlidingDoorActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

ASlidingDoorActor::ASlidingDoorActor()
{
	PrimaryActorTick.bCanEverTick = false;

	DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
	RootComponent = DoorFrame;

	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	Door->SetupAttachment(DoorFrame);

	DoorTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));

}

void ASlidingDoorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASlidingDoorActor::BeginPlay()
{
	Super::BeginPlay();

	ClosedLocation = Door->GetRelativeLocation();

	UpdateFunctionFloat.BindDynamic(this, &ASlidingDoorActor::UpdateTimeline);

	if (DoorCurve)
	{
		DoorTimeline->AddInterpFloat(DoorCurve, UpdateFunctionFloat);
	}
	
}

void ASlidingDoorActor::UpdateTimeline(float Value)
{
	FVector OpenLocation = ClosedLocation + OpenOffset;

	FVector NewLocation = FMath::Lerp(ClosedLocation, OpenLocation, Value);

	Door->SetRelativeLocation(NewLocation);
}

void ASlidingDoorActor::OnIneracted_Implementation(APrototypeCharacter* Character)
{
	if (IInteractionBase::Execute_CanBeInteracted(this))
	{
		IInteractionBase::Execute_HandleInteraction(this, Character);
	}
}

void ASlidingDoorActor::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (!DoorCurve)
		return;

	if (!bIsOpen)
	{
		DoorTimeline->Play();
	}
	else
	{
		DoorTimeline->Reverse();
	}

	bIsOpen = !bIsOpen;
}

bool ASlidingDoorActor::CanBeInteracted_Implementation() const
{
	return true;
}

EInteractionType ASlidingDoorActor::GetInteractionType_Implementation() const
{
	return EInteractionType::Door;
}

bool ASlidingDoorActor::SetBCanInteract(bool IsCanInteract)
{
	bCanInteract = IsCanInteract;
	return bCanInteract;
}

bool ASlidingDoorActor::GetBCanInteract() const
{
	return bCanInteract;
}

void ASlidingDoorActor::HiddenActor()
{
}



#include "Gimmic_CY/Door/DoubleDoor.h"
#include "Components/BoxComponent.h"
#include "NavModifierComponent.h"
#include "NavAreas/NavArea_Default.h"
#include "NavAreas/NavArea_Null.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

ADoubleDoor::ADoubleDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	RightDoor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightDoor"));
	RightDoor->SetupAttachment(CollisionBox);
}

void ADoubleDoor::OpenDoor()
{
	Super::OpenDoor();
	DoorTimelineComp->Play();
	SetBCanInteract(false);
}

void ADoubleDoor::CloseDoor()
{
	Super::CloseDoor();
	DoorTimelineComp->Reverse();

}

void ADoubleDoor::BeginPlay()
{
	Super::BeginPlay();

	UpdateFunctionFloat.BindDynamic(this, &ADoubleDoor::UpdateTimelineComp);

	if (DoorTimelineFloatCurve)
	{
		DoorTimelineComp->AddInterpFloat(DoorTimelineFloatCurve, UpdateFunctionFloat);
	}
	
}

void ADoubleDoor::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (!Character) return;
	if (!bCanInteract) return;

	Activate();
}

void ADoubleDoor::UpdateTimelineComp(float Output)
{
	FVector StartScale(1.f, 1.f, 1.f);
	FVector EndScale(1.f, 0.05f, 1.f);

	FVector NewScale = FMath::Lerp(StartScale, EndScale, Output);

	Mesh->SetRelativeScale3D(NewScale);
	RightDoor->SetRelativeScale3D(NewScale);
}


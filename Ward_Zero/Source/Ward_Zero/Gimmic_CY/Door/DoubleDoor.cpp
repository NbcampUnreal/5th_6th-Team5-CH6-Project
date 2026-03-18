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

	//if (bRequireKeyCard)
	//{
	//	// 카드키 체크
	//	if (!Character->HasKeyCard())
	//	{
	//		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, "Need your Key Card ");
	//		return;
	//	}
	//}


	if (!DoorTimelineFloatCurve) return;

	if (bIsOpen)
	{
		DoorTimelineComp->Reverse();

		NavModifier->SetAreaClass(UNavArea_Default::StaticClass()); // 통과 가능
	}
	else
	{
		DoorTimelineComp->Play();

		NavModifier->SetAreaClass(UNavArea_Null::StaticClass()); // 다시 막힘
	}

	bIsOpen = !bIsOpen;
}

void ADoubleDoor::UpdateTimelineComp(float Output)
{
	FVector StartScale(1.f, 1.f, 1.f);
	FVector EndScale(1.f, 0.05f, 1.f);

	FVector NewScale = FMath::Lerp(StartScale, EndScale, Output);

	Mesh->SetRelativeScale3D(NewScale);
	RightDoor->SetRelativeScale3D(NewScale);
}


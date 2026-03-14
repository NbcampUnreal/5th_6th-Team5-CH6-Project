#include "Gimmic_CY/DoubleDoorActor.h"
#include "Components/BoxComponent.h"
#include "NavModifierComponent.h"
#include "NavAreas/NavArea_Default.h"
#include "NavAreas/NavArea_Null.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

ADoubleDoorActor::ADoubleDoorActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	LeftDoor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftDoor"));
	LeftDoor->SetupAttachment(Root);

	RightDoor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightDoor"));
	RightDoor->SetupAttachment(Root);

	//InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	//InteractionBox->SetupAttachment(Root);

	DoorTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));

	NavModifier = CreateDefaultSubobject<UNavModifierComponent>(TEXT("NavModifier"));
}

void ADoubleDoorActor::BeginPlay()
{
	Super::BeginPlay();

	TimelineUpdate.BindDynamic(this, &ADoubleDoorActor::UpdateDoor);
	NavModifier->SetAreaClass(UNavArea_Null::StaticClass());

	if (DoorCurve)
	{
		DoorTimeline->AddInterpFloat(DoorCurve, TimelineUpdate);
	}
}

void ADoubleDoorActor::UpdateDoor(float Value)
{
	FVector StartScale(1.f, 1.f, 1.f);
	FVector EndScale(1.f, 0.05f, 1.f);

	FVector NewScale = FMath::Lerp(StartScale, EndScale, Value);

	LeftDoor->SetRelativeScale3D(NewScale);
	RightDoor->SetRelativeScale3D(NewScale);
}

void ADoubleDoorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADoubleDoorActor::OnIneractionRangeEntered_Implementation()
{
}

void ADoubleDoorActor::OnIneractionRangeExited_Implementation()
{
}

void ADoubleDoorActor::OnIneracted_Implementation(APrototypeCharacter* Character)
{
	if (IInteractionBase::Execute_CanBeInteracted(this))
	{
		IInteractionBase::Execute_HandleInteraction(this, Character);
	}
}

void ADoubleDoorActor::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (!Character) return;

	if (bRequireKeyCard)
	{
		// 카드키 체크
		if (!Character->HasKeyCard())
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, "Need your Key Card ");
			return;
		}
	}


	if (!DoorCurve) return;

	if (bIsOpen)
	{
		DoorTimeline->Reverse();

		NavModifier->SetAreaClass(UNavArea_Default::StaticClass()); // 통과 가능
	}
	else
	{
		DoorTimeline->Play();

		NavModifier->SetAreaClass(UNavArea_Null::StaticClass()); // 다시 막힘
	}

	bIsOpen = !bIsOpen;
}

bool ADoubleDoorActor::SetBCanInteract(bool IsCanInteract)
{
	bCanInteract = IsCanInteract;
	return bCanInteract;
}

bool ADoubleDoorActor::GetBCanInteract() const
{
	return bCanInteract;
}

void ADoubleDoorActor::HiddenActor()
{
}


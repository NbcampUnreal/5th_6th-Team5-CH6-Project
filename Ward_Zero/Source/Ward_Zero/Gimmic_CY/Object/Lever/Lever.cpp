#include "Gimmic_CY/Object/Lever/Lever.h"

#include "Components/BoxComponent.h"
#include "Gimmic_CY/Interface/InteractionBase.h"
#include "Gimmic_CY/Object/Door/SingleDoor.h"

ALever::ALever()
{
	CollisionBox->SetCanEverAffectNavigation(false);
	CollisionBox->SetGenerateOverlapEvents(false);
	
	Mesh->SetGenerateOverlapEvents(false);
	
	Lamp->SetGenerateOverlapEvents(false);
	
	LeverHandle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeverHandle"));
	LeverHandle->SetupAttachment(RootComponent);
	LeverHandle->SetGenerateOverlapEvents(false);
	
	LeverTimelineComp = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));
	
	PickUpPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PickUpPoint"));
	PickUpPoint->SetupAttachment(LeverHandle);
	PickUpPoint->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	
	
	InteractionCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionCollisionBox"));
	InteractionCollisionBox->SetupAttachment(RootComponent);
	InteractionCollisionBox->SetGenerateOverlapEvents(true);
}

void ALever::BeginPlay()
{
	Super::BeginPlay();
	InitialRotation = LeverHandle->GetRelativeRotation();
	UpdateFunctionFloat.BindDynamic(this,&ALever::UpdateTimelineFunction);
	
	if (LeverTimelineFloatCurve)
	{
		LeverTimelineComp->AddInterpFloat(LeverTimelineFloatCurve, UpdateFunctionFloat);
	}
	
}

void ALever::LeverOpenDoor()
{
	for (ADoorBase* doorActor : DoorsForOpen)
	{
		if (doorActor)
		{
			doorActor->OpenDoor();
		}
	}
}

void ALever::LeverCloseDoor()
{
	for (ADoorBase* doorActor : DoorsForClose)
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

void ALever::UpdateTimelineFunction(float Output)
{
	float NewRoll = FMath::Lerp(StartRoll, TargetRoll, Output);

	FRotator NewRotation = InitialRotation;
	NewRotation.Roll = NewRoll;
	//FRotator NewRotation(0.f, Output, 0.f);
	LeverHandle->SetRelativeRotation(NewRotation);
}

EInteractionType ALever::GetInteractionType_Implementation() const
{
	return EInteractionType::Lever;
}


void ALever::Activate()
{
	Super::Activate();
	LeverTimelineComp->Play();
	LeverOpenDoor();
	LeverUnLockInteraction();
	LeverCloseDoor();
	LeverLockInteraction();
}

void ALever::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (bCanInteract)
	{
		Activate();
	}
}
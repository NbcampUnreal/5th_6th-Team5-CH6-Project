#include "Gimmic_CY/Lever.h"
#include "Gimmic_CY/Base/InteractionBase.h"
#include "Gimmic_CY/Door/SingleDoor.h"

ALever::ALever()
{
	LeverHandle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeverHandle"));
	LeverHandle->SetupAttachment(RootComponent);
	
	LeverTimelineComp = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));
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
	//todo: bIsActivated = SaveManager->CheckActivated(ActorID)
	bool bIsActivated = false;
	
	if (bIsActivated)
	{
		ActivateLever();
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
	return EInteractionType::Door;
}

void ALever::ActivateLever()
{
	LeverTimelineComp->Play();
	SetBCanInteract(false);
}

void ALever::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (bCanInteract)
	{
		ActivateLever();
		LeverOpenDoor();
		LeverUnLockInteraction();
		LeverCloseDoor();
		LeverLockInteraction();
	}
}

FVector ALever::GetIKTargetLocation_Implementation() const
{
	if (LeverHandle)
	{
		return LeverHandle->GetComponentLocation();
	}
	return GetActorLocation();
}
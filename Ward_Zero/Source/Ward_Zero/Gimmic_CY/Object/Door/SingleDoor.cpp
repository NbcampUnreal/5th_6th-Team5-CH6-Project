#include "Gimmic_CY/Object/Door/SingleDoor.h"
#include "NavModifierComponent.h"
#include "NavAreas/NavArea_Default.h"
#include "NavAreas/NavArea_Null.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

ASingleDoor::ASingleDoor()
{
}

void ASingleDoor::BeginPlay()
{
	Super::BeginPlay();

	InitialRotation = Mesh->GetRelativeRotation();
	if (DoorAnimationType == ESingleDoorAnimationType::SingleDoor_Push)
	{
		TargetYaw = -90.f;
	}else
	{
		TargetYaw = 90.f;
	}
}

void ASingleDoor::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (!DoorTimelineFloatCurve || !Character || !bCanInteract)
		return;
	
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	FTimerHandle InteractionTimer;
	TWeakObjectPtr<ASingleDoor> WeakThis(this);
	GetWorld()->GetTimerManager().SetTimer(InteractionTimer, FTimerDelegate::CreateLambda([WeakThis]() {
		WeakThis->Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		}), 1.0f, false);

	FVector DoorLocation = GetActorLocation();
	FVector PlayerLocation = Character->GetActorLocation();

	FVector DoorForward = GetActorForwardVector();
	FVector ToPlayer = (PlayerLocation - DoorLocation).GetSafeNormal();

	float Dot = FVector::DotProduct(DoorForward, ToPlayer);

	

	bIsOpen = true;
	
	Activate();
	

	

}

EInteractionType ASingleDoor::GetInteractionType_Implementation() const
{
	//return EInteractionType::SingleDoor;
	return EInteractionType::Door;
}

ESingleDoorAnimationType ASingleDoor::GetSingleDoorAnimationType() const
{
	return DoorAnimationType;
}

void ASingleDoor::UpdateTimelineComp(float Output)
{
	float NewYaw = FMath::Lerp(0.f, TargetYaw, Output);

	FRotator NewRotation = InitialRotation;
	NewRotation.Yaw += NewYaw;
	Mesh->SetRelativeRotation(NewRotation);
}

ESingleDoorAnimationType ASingleDoor::GetDoorAnimationType()
{
	return DoorAnimationType;
}

void ASingleDoor::OpenDoor()
{
	Super::OpenDoor();
	DoorTimelineComp->Play();
	
}

void ASingleDoor::CloseDoor()
{
	
	Super::CloseDoor();
	
	DoorTimelineComp->Reverse();
	bIsOpen = false;
	
}

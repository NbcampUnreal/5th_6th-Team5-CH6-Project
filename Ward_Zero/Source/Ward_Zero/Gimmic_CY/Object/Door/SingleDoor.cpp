#include "Gimmic_CY/Object/Door/SingleDoor.h"
#include "NavModifierComponent.h"
#include "NavAreas/NavArea_Default.h"
#include "NavAreas/NavArea_Null.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

ASingleDoor::ASingleDoor()
{
	OpenDoorNavModifier = CreateDefaultSubobject<UNavModifierComponent>(TEXT("OpenDoorNavModifier"));
	OpenDoorNavModifier->SetAreaClass(UNavArea_Null::StaticClass());
}

void ASingleDoor::BeginPlay()
{
	Super::BeginPlay();

	InitialRotation = Mesh->GetRelativeRotation();
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

void ASingleDoor::UpdateTimelineComp(float Output)
{
	float NewYaw = FMath::Lerp(0.f, TargetYaw, Output);

	FRotator NewRotation = InitialRotation;
	NewRotation.Yaw += NewYaw;
	Mesh->SetRelativeRotation(NewRotation);
}

void ASingleDoor::OpenDoor()
{
	Super::OpenDoor();
	TargetYaw = -90.f;
	DoorTimelineComp->Play();
	
}

void ASingleDoor::CloseDoor()
{
	
	Super::CloseDoor();
	
	DoorTimelineComp->Reverse();
	bIsOpen = false;
	
}

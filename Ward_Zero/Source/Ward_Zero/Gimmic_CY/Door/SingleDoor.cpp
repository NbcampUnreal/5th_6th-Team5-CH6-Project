#include "Gimmic_CY/Door/SingleDoor.h"
#include "NavModifierComponent.h"
#include "NavAreas/NavArea_Default.h"
#include "NavAreas/NavArea_Null.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

ASingleDoor::ASingleDoor()
{
	//PickUpPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PickUpPoint"));
	//PickUpPoint->SetupAttachment(Mesh);
	//PickUpPoint->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	OpenDoorNavModifier = CreateDefaultSubobject<UNavModifierComponent>(TEXT("OpenDoorNavModifier"));
	OpenDoorNavModifier->SetAreaClass(UNavArea_Null::StaticClass());
}

void ASingleDoor::BeginPlay()
{
	Super::BeginPlay();

	InitialRotation = Mesh->GetRelativeRotation();
	UpdateFunctionFloat.BindDynamic(this, &ASingleDoor::UpdateTimelineComp);

	if (DoorTimelineFloatCurve)
	{
		DoorTimelineComp->AddInterpFloat(DoorTimelineFloatCurve, UpdateFunctionFloat);
	}
	//todo: bIsActivate = SaveManager(ActorID)
	//todo: bIsInteractable = SaveManager->CheckInteractable(ActorId)
	bool bIsActivated = false;
	bool bIsInteractable =  true;
	if (bIsActivated)
	{
		OpenDoor();
	}else if (bIsInteractable)
	{
		bCanInteract = true;
	}
}

void ASingleDoor::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (!DoorTimelineFloatCurve || !Character || !bCanInteract)
		return;

	bCanInteract = false;
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	FTimerHandle InteractionTimer;
	TWeakObjectPtr<ASingleDoor> WeakThis(this);
	GetWorld()->GetTimerManager().SetTimer(InteractionTimer, FTimerDelegate::CreateLambda([WeakThis]() {
		WeakThis->bCanInteract = true;
		WeakThis->Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		}), 1.0f, false);

	//
	FVector DoorLocation = GetActorLocation();
	FVector PlayerLocation = Character->GetActorLocation();

	FVector DoorForward = GetActorForwardVector();
	FVector ToPlayer = (PlayerLocation - DoorLocation).GetSafeNormal();

	float Dot = FVector::DotProduct(DoorForward, ToPlayer);

	//
	//TargetYaw = (Dot >= 0.f) ? 90.f : -90.f;
	//DoorTimelineComp->SetPlayRate(2.0f);

	if (!bIsOpen)
	{
		OpenDoor();
	}

	bIsOpen = !bIsOpen;

}

void ASingleDoor::UpdateTimelineComp(float Output)
{
	float NewYaw = FMath::Lerp(0.f, TargetYaw, Output);

	FRotator NewRotation = InitialRotation;
	NewRotation.Yaw += NewYaw;
	//FRotator NewRotation(0.f, Output, 0.f);
	Mesh->SetRelativeRotation(NewRotation);
}

void ASingleDoor::OpenDoor()
{
	
	Super::OpenDoor();
	
	TargetYaw = -90.f;
	DoorTimelineComp->Play();
	bIsOpen = true;
	bCanInteract = false;
	
}

void ASingleDoor::CloseDoor()
{
	
	Super::CloseDoor();
	
	DoorTimelineComp->Reverse();
	bIsOpen = false;
	
}

//FVector ASingleDoor::GetInteractionTargetLocation_Implementation() const {
//	
//	return PickUpPoint->GetComponentLocation();
//	
//}


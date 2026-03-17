#include "Gimmic_CY/BasicDoorActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NavModifierComponent.h"
#include "NavAreas/NavArea_Default.h"
#include "NavAreas/NavArea_Null.h"
#include "Components/TimelineComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"


ABasicDoorActor::ABasicDoorActor()
{
	PickUpPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PickUpPoint"));
	PickUpPoint->SetupAttachment(Door);
	PickUpPoint->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
}


void ABasicDoorActor::BeginPlay()
{
	Super::BeginPlay();

	InitialRotation = Door->GetRelativeRotation();
	UpdateFunctionFloat.BindDynamic(this, &ABasicDoorActor::UpdateTimelineComp);
}

void ABasicDoorActor::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (!DoorTimelineFloatCurve || !Character || !bCanInteract)
		return;

	bCanInteract = false;
	Door->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	FTimerHandle InteractionTimer;
	TWeakObjectPtr<ABasicDoorActor> WeakThis(this);
	GetWorld()->GetTimerManager().SetTimer(InteractionTimer, FTimerDelegate::CreateLambda([WeakThis]() {
		WeakThis->bCanInteract = true;
		WeakThis->Door->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
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
		TargetYaw = (Dot >= 0.f) ? 90.f : -90.f;

		NavModifier->SetAreaClass(UNavArea_Default::StaticClass()); // 통과 가능
		DoorTimelineComp->Play();
	}
	else
	{
		DoorTimelineComp->Reverse();

		NavModifier->SetAreaClass(UNavArea_Null::StaticClass()); // 다시 막힘
	}

	bIsOpen = !bIsOpen;

}

FVector ABasicDoorActor::GetInteractionTargetLocation_Implementation() const {
	return PickUpPoint->GetComponentLocation();
}

void ABasicDoorActor::UpdateTimelineComp(float Output)
{
	float NewYaw = FMath::Lerp(0.f, TargetYaw, Output);

	FRotator NewRotation = InitialRotation;
	NewRotation.Yaw += NewYaw;

	//FRotator NewRotation(0.f, Output, 0.f);
	Door->SetRelativeRotation(NewRotation);

}

#include "Gimmic_CY/Base/DoorBase.h"
#include "NavModifierComponent.h"
#include "NavAreas/NavArea_Default.h"
#include "NavAreas/NavArea_Null.h"
#include "Components/BoxComponent.h"

ADoorBase::ADoorBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Lamp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Lamp"));
	Lamp->SetupAttachment(CollisionBox);

	DoorTimelineComp = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));

	NavModifier = CreateDefaultSubobject<UNavModifierComponent>(TEXT("NavModifier"));

	PickUpPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PickUpPoint"));
	PickUpPoint->SetupAttachment(Mesh);
	PickUpPoint->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	NavModifier->SetAreaClass(UNavArea_Null::StaticClass());
	
	

}

void ADoorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADoorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

EInteractionType ADoorBase::GetInteractionType_Implementation() const
{
	return EInteractionType::Door;
}

void ADoorBase::ChangeColorLampRed_Implementation()
{

}

void ADoorBase::ChangeColorLampGreen_Implementation()
{

}

void ADoorBase::OpenDoor()
{
	if (bIsOpen)
		return;
	NavModifier->SetAreaClass(UNavArea_Default::StaticClass());
}

void ADoorBase::CloseDoor()
{
	if (!bIsOpen)
		return;
	NavModifier->SetAreaClass(UNavArea_Null::StaticClass());
}

void ADoorBase::UpdateTimelineComp(float Value)
{
	
}

FVector ADoorBase::GetInteractionTargetLocation_Implementation() const {
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, "Enable PickupPoint");
	return PickUpPoint->GetComponentLocation();
}

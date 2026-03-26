#include "Gimmic_CY/Object/Door/DoorBase.h"
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
	NavModifier->SetAreaClass(UNavArea_Null::StaticClass());
	NavModifier->SetCanEverAffectNavigation(true);
	PickUpPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PickUpPoint"));
	PickUpPoint->SetupAttachment(Mesh);
	PickUpPoint->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	
	
	
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

void ADoorBase::OpenDoor()
{
	NavModifier->SetAreaClass(UNavArea_Default::StaticClass());
}

void ADoorBase::CloseDoor()
{
	NavModifier->SetAreaClass(UNavArea_Null::StaticClass());
}



void ADoorBase::UpdateTimelineComp(float Value)
{
	
}

void ADoorBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	bIsOpen = false;
}

void ADoorBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	UpdateFunctionFloat.BindDynamic(this, &ADoorBase::UpdateTimelineComp);

	if (DoorTimelineFloatCurve)
	{
		DoorTimelineComp->AddInterpFloat(DoorTimelineFloatCurve, UpdateFunctionFloat);
		
	}
}

FVector ADoorBase::GetInteractionTargetLocation_Implementation() const {
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, "Enable PickupPoint");
	return PickUpPoint->GetComponentLocation();
}

void ADoorBase::Activate()
{
	Super::Activate();
	OpenDoor();
}

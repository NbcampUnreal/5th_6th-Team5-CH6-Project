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
}

void ADoorBase::BeginPlay()
{
	Super::BeginPlay();
	NavModifier->SetAreaClass(UNavArea_Null::StaticClass());
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

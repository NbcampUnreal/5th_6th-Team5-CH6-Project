#include "Gimmic_CY/DoorBase.h"
#include "Components/BoxComponent.h"
#include "NavModifierComponent.h"
#include "NavAreas/NavArea_Default.h"
#include "NavAreas/NavArea_Null.h"
#include "Components/TimelineComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

ADoorBase::ADoorBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetBoxExtent(FVector(10.0f, 10.0f, 10.0f));
	SetRootComponent(CollisionBox);;
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	Door->SetupAttachment(CollisionBox);
	Door->SetCollisionResponseToChannels(ECR_Block);

	Lamp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Lamp"));
	Lamp->SetupAttachment(CollisionBox);

	DoorTimelineComp = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));

	NavModifier = CreateDefaultSubobject<UNavModifierComponent>(TEXT("NavModifier"));
	//NavModifier->SetupAttachment(Scene);
}

void ADoorBase::BeginPlay()
{
	Super::BeginPlay();
	NavModifier->SetAreaClass(UNavArea_Null::StaticClass());

	if (DoorTimelineFloatCurve)
	{
		DoorTimelineComp->AddInterpFloat(DoorTimelineFloatCurve, UpdateFunctionFloat);
	}
}

// Called every frame
void ADoorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADoorBase::OnIneractionRangeEntered_Implementation()
{
}

void ADoorBase::OnIneractionRangeExited_Implementation()
{
}

void ADoorBase::OnIneracted_Implementation(APrototypeCharacter* Character)
{
	if (IInteractionBase::Execute_CanBeInteracted(this))
	{
		IInteractionBase::Execute_HandleInteraction(this, Character);
	}
}

void ADoorBase::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
}

EInteractionType ADoorBase::GetInteractionType_Implementation() const
{
	return EInteractionType::Door;
}

bool ADoorBase::SetBCanInteract(bool IsCanInteract)
{
	bCanInteract = IsCanInteract;
	if (bCanInteract)
	{
		ChangeColorLampGreen_Implementation();
	}
	else
	{
		ChangeColorLampRed_Implementation();
	}
	return bCanInteract;
}

bool ADoorBase::GetBCanInteract() const
{
	return bCanInteract;
}

void ADoorBase::PostActorCreated()
{
	Super::PostActorCreated();

	// 액터가 에디터에 배치되거나 스폰될 때 최초 1회만 GUID 생
	if (!ActorID.IsValid())
	{
		ActorID = FGuid::NewGuid();
		UE_LOG(LogTemp, Warning, TEXT("New Item ID Generated: %s"), *ActorID.ToString());
	}
}

void ADoorBase::HiddenActor()
{
}

FVector ADoorBase::GetInteractionTargetLocation() const
{
	return FVector();
}

FGuid ADoorBase::GetActorID() const
{
	return ActorID;
}

void ADoorBase::SaveActorState(UWardSaveGame* SaveData)
{

}

void ADoorBase::LoadActorState(UWardSaveGame* SaveData)
{

}

void ADoorBase::ChangeColorLampRed_Implementation()
{

}

void ADoorBase::ChangeColorLampGreen_Implementation()
{

}

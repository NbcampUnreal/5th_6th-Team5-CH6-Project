#include "Gimmic_CY/SafeActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

ASafeActor::ASafeActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SafeBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SafeBody"));
    RootComponent = SafeBody;

    Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
    Pivot->SetupAttachment(SafeBody);

    Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
    Door->SetupAttachment(Pivot);

    InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
    InteractionBox->SetupAttachment(SafeBody);

    SafeTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("SafeTimeline"));

}

void ASafeActor::BeginPlay()
{
	Super::BeginPlay();
	
    InitialRotation = Pivot->GetRelativeRotation();

    SafeUpdateFunctionFloat.BindDynamic(this, &ASafeActor::UpdateSafeRotation);

    if (OpenCurve)
    {
        SafeTimeline->AddInterpFloat(OpenCurve, SafeUpdateFunctionFloat);
    }

    InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &ASafeActor::OnBeginOverlap);
    InteractionBox->OnComponentEndOverlap.AddDynamic(this, &ASafeActor::OnEndOverlap);
}

void ASafeActor::UpdateSafeRotation(float Alpha)
{
    float NewYaw = FMath::Lerp(0.f, TargetYaw, Alpha);

    FRotator NewRot = InitialRotation;
    NewRot.Yaw += NewYaw;

    Pivot->SetRelativeRotation(NewRot);
}

void ASafeActor::OnIneractionRangeEntered()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, "I am an ineractable");
	// À§Á¬ Ç¥½Ã µî
}

void ASafeActor::OnIneractionRangeExited()
{
	// À§Á¬ ¼û±è µî
}

void ASafeActor::OnIneracted(APrototypeCharacter* Character)
{
	if (CanBeInteracted())
	{
		HandleInteraction(Character);
	}
}

void ASafeActor::HandleInteraction(APrototypeCharacter* Character)
{
    if (!SafeTimeline)
        return;

    if (bIsOpened)
    {
        SafeTimeline->Reverse();
        bIsOpened = false;
    }
    else
    {
        SafeTimeline->PlayFromStart();
        bIsOpened = true;
    }
}

void ASafeActor::OnBeginOverlap(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    OnIneractionRangeEntered();
}

void ASafeActor::OnEndOverlap(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32)
{
    OnIneractionRangeExited();
}



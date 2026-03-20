#include "Gimmic_CY/Door/SafeActor.h"

ASafeActor::ASafeActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
    Pivot->SetupAttachment(Mesh);

    Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
    Door->SetupAttachment(Pivot);
}

void ASafeActor::BeginPlay()
{
    Super::BeginPlay();

    InitialRotation = Pivot->GetRelativeRotation();

    UpdateFunctionFloat.BindDynamic(this, &ASafeActor::UpdateTimelineComp);

    if (DoorTimelineFloatCurve)
    {
        DoorTimelineComp->AddInterpFloat(DoorTimelineFloatCurve, UpdateFunctionFloat);
    }
   
}

void ASafeActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if (bVanishMagic)
    {
        Door->SetVisibility(false);
    }else
    {
        Door->SetVisibility(true);
    }
}

void ASafeActor::UpdateTimelineComp(float Output)
{
    float NewYaw = FMath::Lerp(0.f, TargetYaw, Output);

    FRotator NewRot = InitialRotation;
    NewRot.Yaw += NewYaw;

    Pivot->SetRelativeRotation(NewRot);
}

void ASafeActor::DoorVanishMagic()
{
    bVanishMagic = !bVanishMagic;
    OnConstruction(GetActorTransform());
}

void ASafeActor::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
    if (!bCanInteract)
        return;
    Activate();
    for (AActor* Item : Items)
    {
        IInteractionBase* interactionBaseItem = Cast<IInteractionBase>(Item);
        if (interactionBaseItem)
        {
            interactionBaseItem->SetBCanInteract(true);
        }
    }
}

void ASafeActor::Activate()
{
    Super::Activate();
    if (!DoorTimelineComp)
        return;

    if (!bIsOpen)
    {
        DoorTimelineComp->Play();
        bIsOpen = true;
        Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
    }
    SetBCanInteract(false);
}

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

    for (AActor* Item : Items)
    {
        IInteractionBase* interactionBaseItem = Cast<IInteractionBase>(Item);
        if (interactionBaseItem)
        {
            interactionBaseItem->SetBCanInteract(false);
        }
    }
}

void ASafeActor::UpdateTimelineComp(float Output)
{
    float NewYaw = FMath::Lerp(0.f, TargetYaw, Output);

    FRotator NewRot = InitialRotation;
    NewRot.Yaw += NewYaw;

    Pivot->SetRelativeRotation(NewRot);
}

void ASafeActor::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
    if (!DoorTimelineComp)
        return;

    if (!bIsOpen)
    {
        DoorTimelineComp->Play();
        bIsOpen = true;

        for (AActor* Item : Items)
        {
            IInteractionBase* interactionBaseItem = Cast<IInteractionBase>(Item);
            if (interactionBaseItem)
            {
                interactionBaseItem->SetBCanInteract(true);
            }
        }
        Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
    }
}

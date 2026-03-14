#include "Gimmic_CY/SafeActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AKeyCard.h"
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

    //InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
    //InteractionBox->SetupAttachment(SafeBody);

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

    for (AActor* Item : Items)
    {
        IInteractionBase* interactionBaseItem = Cast<IInteractionBase>(Item);
        if (interactionBaseItem)
        {
            interactionBaseItem->SetBCanInteract(false);            
        }
    }

    //InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &ASafeActor::OnBeginOverlap);
    //InteractionBox->OnComponentEndOverlap.AddDynamic(this, &ASafeActor::OnEndOverlap);
}

void ASafeActor::UpdateSafeRotation(float Alpha)
{
    float NewYaw = FMath::Lerp(0.f, TargetYaw, Alpha);

    FRotator NewRot = InitialRotation;
    NewRot.Yaw += NewYaw;

    Pivot->SetRelativeRotation(NewRot);
}

void ASafeActor::OnIneractionRangeEntered_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, "I am an ineractable");
	// À§Á¬ Ç¥½Ã µî
}

void ASafeActor::OnIneractionRangeExited_Implementation()
{
	// À§Á¬ ¼û±è µî
}

void ASafeActor::OnIneracted_Implementation(APrototypeCharacter* Character)
{

    if (IInteractionBase::Execute_CanBeInteracted(this))
    {
        IInteractionBase::Execute_HandleInteraction(this, Character);
    }
}

void ASafeActor::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
    if (!SafeTimeline)
        return;

    if (!bIsOpened)
    {
        SafeTimeline->Play();
        bIsOpened = true;

        for (AActor* Item : Items)
        {
            IInteractionBase* interactionBaseItem = Cast<IInteractionBase>(Item);
            if (interactionBaseItem)
            {
                interactionBaseItem->SetBCanInteract(true);
            }
        }
        SafeBody->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
    }
}

void ASafeActor::OnBeginOverlap(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    if (Cast<APrototypeCharacter>(OtherActor))
    {
        IInteractionBase::Execute_OnIneractionRangeEntered(this);
    }
}

void ASafeActor::OnEndOverlap(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32)
{
    IInteractionBase::Execute_OnIneractionRangeExited(this);
}

bool ASafeActor::SetBCanInteract(bool IsCanInteract)
{
    bCanInteract = IsCanInteract;
    return bCanInteract;
}

bool ASafeActor::GetBCanInteract() const
{
    return bCanInteract;
}

void ASafeActor::HiddenActor()
{
}



#include "Gimmic_CY/Object/Door/SafeActor.h"

#include "NavModifierComponent.h"
#include "Gimmic_CY/Items/ItemBase.h"
#include "Kismet/GameplayStatics.h"
#include "NavAreas/NavArea_Null.h"

#if WITH_EDITOR
#include "EngineUtils.h"
#endif

ASafeActor::ASafeActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
    Pivot->SetupAttachment(Mesh);

    Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
    Door->SetupAttachment(Pivot);
    
    PullPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PullPoint"));
    PullPoint->SetupAttachment(Door);
    
    
}

void ASafeActor::BeginPlay()
{
    Super::BeginPlay();

    InitialRotation = Pivot->GetRelativeRotation();
    
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
        //Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Overlap);
    }
    NavModifier->SetAreaClass(UNavArea_Null::StaticClass());
    SetBCanInteract(false);
}

EInteractionType ASafeActor::GetInteractionType_Implementation() const
{
    //return EInteractionType::SingleDoor;
    return EInteractionType::Door;
}

ESingleDoorAnimationType ASafeActor::GetSingleDoorAnimationType() const
{
    return DoorAnimationType;
}
#if WITH_EDITOR
void ASafeActor::DoorVanishMagic()
{
    bVanishMagic = !bVanishMagic;
    OnConstruction(GetActorTransform());
}

void ASafeActor::SettingItemInSafeBox(ASafeActor* Obj)
{
    for (AActor* Item : Obj->Items)
    {
        AItemBase* IB = Cast<AItemBase>(Item);
        if (IB)
        {
            IB->bDefaultInteractable = false;
        }
    }
}

void ASafeActor::RegistAllItemWithTag()
{
    TArray<AActor*> ItemsWithTag;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(),ItemTag,ItemsWithTag);
    
    for (AActor* Item : ItemsWithTag)
    {
        RegistItem(Item);
       
    }
}

void ASafeActor::RegistItem(AActor* item)
{
    
    AItemBase* IB = Cast<AItemBase>(item);
    if (IB)
    {
        Items.Add(IB);
        IB->bDefaultInteractable = false;
    }
}

void ASafeActor::SettingItemAllSafeBox()
{
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<ASafeActor> It(World); It; ++It)
        {
            ASafeActor* Obj = *It;
            if (Obj)
            {
                SettingItemInSafeBox(Obj);
            }
        }
    }
    
}
#endif
#include "Gimmic_CY/Object/Door/SafeActor.h"

#include "NavModifierComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Components/BoxComponent.h"
#include "Gimmic_CY/Items/ItemBase.h"
#include "Kismet/GameplayStatics.h"
#include "NavAreas/NavArea_Null.h"
#include "Components/CapsuleComponent.h"


#if WITH_EDITOR
#include "EngineUtils.h"
#endif

ASafeActor::ASafeActor()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionBox->SetGenerateOverlapEvents(false);
    Mesh->SetGenerateOverlapEvents(false);
    
    Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
    Pivot->SetupAttachment(Mesh);
    
    Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
    Door->SetupAttachment(Pivot);
    Door->SetGenerateOverlapEvents(false);
    
    PullPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PullPoint"));
    PullPoint->SetupAttachment(Door);
    
    InteractionCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionCollisionBox"));
    InteractionCollisionBox->SetupAttachment(RootComponent);
    InteractionCollisionBox->SetGenerateOverlapEvents(true);
    
    
}

void ASafeActor::BeginPlay()
{
    Super::BeginPlay();

    InitialRotation = Pivot->GetRelativeRotation();
    if (Door)
    {
        InitialRotation = Door->GetRelativeRotation();
        Door->OnComponentBeginOverlap.AddDynamic(this, &ASafeActor::OnOverLapBegin);
        Door->OnComponentEndOverlap.AddDynamic(this, &ASafeActor::OnOverLapEnd);
        UMaterialInterface* BaseM = Door->GetMaterial(0);
        if (BaseM)
        {
            DynamicMaterial = UMaterialInstanceDynamic::Create(BaseM,this);
            Door->SetMaterial(0, DynamicMaterial);
        }
		
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
        Door->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Overlap);

        FTimerHandle InteractionTimer;
        TWeakObjectPtr<ASafeActor> WeakThis(this);
        GetWorld()->GetTimerManager().SetTimer(InteractionTimer, FTimerDelegate::CreateLambda([WeakThis]() {
            WeakThis->Door->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Block);
            }), 1.0f, false);
        DoorTimelineComp->Play();
        bIsOpen = true;
        //Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Overlap);
    }
    NavModifier->SetAreaClass(UNavArea_Null::StaticClass());
    SetBCanInteract(false);
}

void ASafeActor::OnOverLapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
    class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && (OtherActor != this))
    {
        APrototypeCharacter* Player = Cast<APrototypeCharacter>(OtherActor);
        if (Player && OtherComp == Player->GetCapsuleComponent())
        {
            DynamicMaterial->SetScalarParameterValue(FName("OpacityParam"),0.3f);
        }
    }
}

void ASafeActor::OnOverLapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
    class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && (OtherActor != this))
    {
        APrototypeCharacter* Player = Cast<APrototypeCharacter>(OtherActor);
        if (Player && OtherComp == Player->GetCapsuleComponent())
        {
            DynamicMaterial->SetScalarParameterValue(FName("OpacityParam"),1.0f);
        }
    }
}

EInteractionType ASafeActor::GetInteractionType_Implementation() const
{
    return EInteractionType::SafeBox;
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
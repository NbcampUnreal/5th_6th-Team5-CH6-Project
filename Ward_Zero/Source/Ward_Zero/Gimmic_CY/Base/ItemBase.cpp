#include "ItemBase.h"

#include "WardGameInstanceSubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "UI_KWJ/Save/WardSaveGame.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetBoxExtent(FVector(10.0f, 10.0f, 10.0f));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Mesh->SetupAttachment(CollisionBox);
	

	PickUpPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PickUpPoint"));
	PickUpPoint->SetupAttachment(Mesh); 
	PickUpPoint->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	
	InteractWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractWidget"));
	InteractWidget->SetupAttachment(RootComponent);
	InteractWidget->SetWidgetSpace(EWidgetSpace::Screen);
	InteractWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));
	InteractWidget->SetDrawSize(FVector2D(200.0f, 50.0f));
	InteractWidget->SetVisibility(false);
	
	static ConstructorHelpers::FClassFinder<UUserWidget> InteractWidgetClass(TEXT("/Game/Gimmick/Gimmick_CY/Widget/WB_PressE.WB_PressE_C"));
	
	if (InteractWidgetClass.Succeeded())
	{
		InteractWidget->SetWidgetClass(InteractWidgetClass.Class);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to find InteractWidget class!"));
	}
	
	
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();
	bGamePlay = true;
	SetBCanInteract(bDefaultInteractable);
	//todo: bIsActivated = SaveManager->CheckActivated(ActorID)
	//todo: bIsInterActable = SaveManager->CheckInterActable(ActorID)
	/*bool bIsActivated = false;
	bool bIsInteractable = true;
	if (bIsActivated)
	{
		HiddenActor();
	}else
	{
		SetBCanInteract(bIsInteractable);
	}*/
}

void AItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItemBase::OnIneractionRangeEntered_Implementation()
{
}

void AItemBase::OnIneractionRangeExited_Implementation()
{
}

void AItemBase::OnIneracted_Implementation(APrototypeCharacter* Character)
{
	if (IInteractionBase::Execute_CanBeInteracted(this))
	{
		IInteractionBase::Execute_HandleInteraction(this, Character);
	}
}

void AItemBase::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (!bCanInteract)
		return;

	bCollected = true;

	HiddenActor();

	SaveActorState();
	
}

EInteractionType AItemBase::GetInteractionType_Implementation() const
{
	return EInteractionType();
}

bool AItemBase::SetBCanInteract(bool IsCanInteract)
{
	bCanInteract = IsCanInteract;
	SaveActorState();
	return bCanInteract;
}

bool AItemBase::GetBCanInteract() const
{
	return bCanInteract;
}

void AItemBase::SaveActorState() const
{
	if (!bGamePlay)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UWardGameInstanceSubsystem* WardGISubSys = GI->GetSubsystem<UWardGameInstanceSubsystem>())
			{
				WardGISubSys->SetObjectState(ActorID,bActivated,bCanInteract);
			}
		}
	}
}

void AItemBase::HiddenActor()
{
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetVisibility(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetBCanInteract(false);
	bActivated = true;
}

void AItemBase::PostActorCreated()
{
	Super::PostActorCreated();

	// ���Ͱ� �����Ϳ� ��ġ�ǰų� ������ �� ���� 1ȸ�� GUID ����
	if (!ActorID.IsValid())
	{
		ActorID = FGuid::NewGuid();
		UE_LOG(LogTemp, Warning, TEXT("New Item ID Generated: %s"), *ActorID.ToString());
	}
}

void AItemBase::ShowPressEWidget_Implementation()
{
	IInteractionBase::ShowPressEWidget_Implementation();
	InteractWidget->SetVisibility(true);
}

void AItemBase::HidePressEWidget_Implementation()
{
	IInteractionBase::HidePressEWidget_Implementation();
	InteractWidget->SetVisibility(false);
}


FVector AItemBase::GetInteractionTargetLocation_Implementation() const
{
	return GetActorLocation();
}


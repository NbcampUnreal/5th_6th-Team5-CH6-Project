#include "Gimmic_CY/Base/ObjectBase.h"
#include "Components/BoxComponent.h"

AObjectBase::AObjectBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetBoxExtent(FVector(10.0f, 10.0f, 10.0f));
	SetRootComponent(CollisionBox);;
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(CollisionBox);
	Mesh->SetCollisionResponseToChannels(ECR_Block);
}

void AObjectBase::BeginPlay()
{
	Super::BeginPlay();
	
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

void AObjectBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AObjectBase::OnIneractionRangeEntered_Implementation()
{
}

void AObjectBase::OnIneractionRangeExited_Implementation()
{
}

void AObjectBase::OnIneracted_Implementation(APrototypeCharacter* Character)
{
	if (IInteractionBase::Execute_CanBeInteracted(this))
	{
		IInteractionBase::Execute_HandleInteraction(this, Character);
	}
}

void AObjectBase::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
}

EInteractionType AObjectBase::GetInteractionType_Implementation() const
{
	return EInteractionType();
}

bool AObjectBase::SetBCanInteract(bool IsCanInteract)
{
	bCanInteract = IsCanInteract;
	//todo: SaveManager->SetActorInteractable(ActorId,bCanInteract)
	return bCanInteract;
}

bool AObjectBase::GetBCanInteract() const
{
	return bCanInteract;
}

void AObjectBase::PostActorCreated()
{
	Super::PostActorCreated();

	//���Ͱ� �����Ϳ� ��ġ�ǰų� ������ �� ���� 1ȸ�� GUID ��
	if (!ActorID.IsValid())
	{
		ActorID = FGuid::NewGuid();
		UE_LOG(LogTemp, Warning, TEXT("New Item ID Generated: %s"), *ActorID.ToString());
	}
}

void AObjectBase::Activate()
{
	//todo: SaveManager->SetActorActivated(ActorID)
	SetBCanInteract(false);
}

FVector AObjectBase::GetInteractionTargetLocation_Implementation() const
{
	return GetActorLocation();
}


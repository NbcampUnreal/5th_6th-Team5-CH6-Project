#include "Gimmic_CY/Object/ObjectBase.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "WardGameInstanceSubsystem.h"
#include "UI_KWJ/Save/WardSaveGame.h"

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
	
	InteractWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractWidget"));
	InteractWidget->SetupAttachment(RootComponent);
	InteractWidget->SetWidgetSpace(EWidgetSpace::Screen);
	InteractWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
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

void AObjectBase::BeginPlay()
{
	Super::BeginPlay();
	//SetBCanInteract(bDefaultInteractable);
	bCanInteract = bDefaultInteractable;
	bGamePlay = true;
	
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UWardGameInstanceSubsystem* WardGISubSys = GI->GetSubsystem<UWardGameInstanceSubsystem>())
			{
				if (WardGISubSys->HasObjectState(ActorID))
				{
					FObjectSaveData ActorData = WardGISubSys->GetObjectState(ActorID);
					if (ActorData.bActive)
					{
						Activate();
					}else if (ActorData.bCanInteract)
					{
						SetBCanInteract(true);
					}else
					{
						SetBCanInteract(false);
					}
				}
			}
		}
	}
	
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
	SaveActorState();
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



void AObjectBase::ShowPressEWidget_Implementation()
{
	IInteractionBase::ShowPressEWidget_Implementation();
	InteractWidget->SetVisibility(true);
}

void AObjectBase::HidePressEWidget_Implementation()
{
	IInteractionBase::HidePressEWidget_Implementation();
	InteractWidget->SetVisibility(false);
}

void AObjectBase::SaveActorState() const
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


void AObjectBase::Activate()
{
	bActivated = true;
	SetBCanInteract(false);
}

FVector AObjectBase::GetInteractionTargetLocation_Implementation() const
{
	return GetActorLocation();
}


#include "ItemBase.h"

#include "WardGameInstanceSubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Gimmic_CY/Object/Door/SafeActor.h"
#include "Kismet/GameplayStatics.h"
#include "UI_KWJ/InteractionHint/InteractionHintSubsystem.h"
#include "UI_KWJ/ItemNotify/ItemNotifySubsystem.h"
#include "UI_KWJ/PickupNotify/PickupNotifySubsystem.h"
#include "UI_KWJ/Reading/DocumentSubsystem.h"
#include "UI_KWJ/Save/WardSaveGame.h"
#if WITH_EDITOR
#include "EngineUtils.h"
#endif

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetBoxExtent(FVector(10.0f, 10.0f, 10.0f));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionBox->SetCollisionResponseToChannels(ECR_Overlap);

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
	
	static ConstructorHelpers::FClassFinder<UUserWidget> InteractWidgetClass(TEXT("/Game/UI/save/WBP_InteractPrompt.WBP_InteractPrompt_C"));
	
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
	//SetBCanInteract(bDefaultInteractable);
	bCanInteract = bDefaultInteractable;
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
						HiddenActor();
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
	if (bHasDoc)
	{
		ShowDocument();
	}
	if (bHasSubtitle)
	{
		ShowSubtitle();
	}
	
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UPickupNotifySubsystem* PickUpNotifySubsys = LP->GetSubsystem<UPickupNotifySubsystem>())
			{
				
				PickUpNotifySubsys->ShowPickup(FText::FromString(PickUpText));
				
			}
			if (UItemNotifySubsystem* NotifySys = PC->GetLocalPlayer()->GetSubsystem<UItemNotifySubsystem>())
			{
				NotifySys->ShowItemNotifyByIndex(DocIdx);
			}
			
		}
	}
	

	HiddenActor();
	
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
	bActivated = true;
	SetBCanInteract(false);
	
}

void AItemBase::ShowSubtitle() const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UInteractionHintSubsystem* InteractionHintSubsys = LP->GetSubsystem<UInteractionHintSubsystem>())
			{
				InteractionHintSubsys->ShowHintWithText(FText::FromString(Subtitle));
			}
		}
	}
}

void AItemBase::ShowDocument() const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UDocumentSubsystem* DocSys = LP->GetSubsystem<UDocumentSubsystem>())
			{
				DocSys->OpenDocumentByIndex(DocIdx);
			}
		}
	}
}

void AItemBase::PostActorCreated()
{
	Super::PostActorCreated();
	
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

FVector AItemBase::GetIKTargetLocation_Implementation() const
{
	// 일반 아이템은 기존 상호작용 타겟 위치를 그대로 손 위치로 사용합니다.
	return GetInteractionTargetLocation();
}

#if WITH_EDITOR
void AItemBase::PostDuplicate(EDuplicateMode::Type DuplicateMode)
{
	Super::PostDuplicate(DuplicateMode);

	if (DuplicateMode == EDuplicateMode::Normal)
	{
		ActorID = FGuid::NewGuid();
		UE_LOG(LogTemp, Warning, TEXT("Duplicated Item ID Generated: %s"), *ActorID.ToString());
	}
}

void AItemBase::PostEditImport()
{
	Super::PostEditImport();
	
	ActorID = FGuid::NewGuid();
	UE_LOG(LogTemp, Warning, TEXT("Imported Item ID Generated: %s"), *ActorID.ToString());
}
void AItemBase::RegenerateAllObjectIDsInLevel()
{
	if (UWorld* World = GetWorld())
	{
		int32 Count = 0;
		for (TActorIterator<AItemBase> It(World); It; ++It)
		{
			AItemBase* Obj = *It;
			if (Obj)
			{
				Obj->ActorID = FGuid::NewGuid();
				
				Obj->Modify(); 
				
				UE_LOG(LogTemp, Warning, TEXT("Regenerated ID for %s: %s"), *Obj->GetName(), *Obj->ActorID.ToString());
				Count++;
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("Total %d Object IDs have been successfully regenerated."), Count);
	}
}

void AItemBase::FindClosestSafeBoxAndRegist()
{
	if (UWorld* World = GetWorld())
	{
		FVector ItemLocation = GetActorLocation();
		float ClosestDistance = -1.0f;
		ASafeActor* TargetSafeActor = nullptr;
		for (TActorIterator<ASafeActor> It(World); It; ++It)
		{
			ASafeActor* Obj = *It;
			if (Obj)
			{
				FVector SafeBoxLocation = Obj->GetActorLocation();
				float dist = FVector::Dist(ItemLocation, SafeBoxLocation);
				if (ClosestDistance <0)
				{
					ClosestDistance = dist;
					TargetSafeActor = Obj;
				}else if (dist < ClosestDistance)
				{
					ClosestDistance = dist;
					TargetSafeActor = Obj;
				}
			}
		}
		if (TargetSafeActor)
		{
			TargetSafeActor->RegistItem(this);
		}
		
	}
}
#endif

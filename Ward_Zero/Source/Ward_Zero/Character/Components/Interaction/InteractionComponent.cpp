#include "Character/Components/Interaction/InteractionComponent.h"
#include "Components/SphereComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Gimmic_CY/Interface/InteractionBase.h"
#include "UI_KWJ/Save/SaveSubsystem.h"
#include "MotionWarpingComponent.h"
#include "Character/Data/AnimData/CharacterAnimData.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Gimmic_CY/Object/Lever/Lever.h"
#include "Gimmic_CY/Items/ItemBase.h"
#include "Gimmic_CY/Object/Door/SingleDoor.h"
#include "Gimmic_CY/Object/Door/SafeActor.h"
#include "Gimmic_CY/Interface/InteractionBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "UI_KWJ/InteractionHint/InteractionHintSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Gimmic_CY/Object/Door/SlidingDoor.h"
#include "Gimmic_CY/Object/Door/DoubleDoor.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
}

void UInteractionComponent::Initialize(APrototypeCharacter* InCharacter)
{
	OwnerCharacter = InCharacter;

	if (!InCharacter) return;

	InteractionSphere->UnregisterComponent();
	InteractionSphere->AttachToComponent(OwnerCharacter->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	InteractionSphere->RegisterComponent();

	InteractionSphere->SetSphereRadius(100.0f);
	InteractionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	// 델리게이트 연결
	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &UInteractionComponent::OnInteractableBeganOverlap);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &UInteractionComponent::OnInteractableEndedOverlap);
}

void UInteractionComponent::TryInteract()
{
	if (!OwnerCharacter || !InteractionSphere) return;
	if (OwnerCharacter->GetIsReloading() || OwnerCharacter->IsFiring() || OwnerCharacter->IsEquipping()) return;
	TArray<AActor*> OverlappingActors;
	InteractionSphere->GetOverlappingActors(OverlappingActors);

	AActor* ClosestInteractable = nullptr;
	AActor* ClosetLockedDoor = nullptr; 

	float MinInteractDistSq = MAX_FLT;
	float MinLockedDistSq = MAX_FLT;

	FVector PlayerLocation = OwnerCharacter->GetActorLocation();

	for (AActor* Actor : OverlappingActors)
	{
		if (!IsValid(Actor) || Actor == OwnerCharacter) continue;
		if (!Actor->GetClass()->ImplementsInterface(UInteractionBase::StaticClass())) continue;

		IInteractionBase* InteractInterface = Cast<IInteractionBase>(Actor);
		if (!InteractInterface) continue; 

		float DistSq = FVector::DistSquared(PlayerLocation, Actor->GetActorLocation());

		// 상호작용 가능
		if (InteractInterface->GetBCanInteract())
		{
			if (DistSq < MinInteractDistSq)
			{
				MinInteractDistSq = DistSq;
				ClosestInteractable = Actor;
			}
		}
		// 상호작용 불가능 & 타입 Door의 경우  
		else 
		{
			EInteractionType Type = IInteractionBase::Execute_GetInteractionType(Actor);
			if (Type == EInteractionType::Door || Type == EInteractionType::SingleDoor)
			{
				if (DistSq < MinLockedDistSq)
				{
					MinLockedDistSq = DistSq;
					ClosetLockedDoor = Actor;
				}
			}
		}
	}
	
	// 상호작용 실행 
	if (ClosestInteractable)
	{
		EInteractionType Type = IInteractionBase::Execute_GetInteractionType(ClosestInteractable);

		if (Type == EInteractionType::Door) HandleDoorInteraction(ClosestInteractable);
		else if (Type == EInteractionType::Ammo || Type == EInteractionType::Heal || Type == EInteractionType::Key)
			HandleItemInteraction(ClosestInteractable);
		else if (Type == EInteractionType::Lever)
			HandleLeverInteraction(ClosestInteractable);
		else
		{
			IInteractionBase::Execute_HidePressEWidget(ClosestInteractable);
			IInteractionBase::Execute_OnIneracted(ClosestInteractable, OwnerCharacter);
		}
		return;
	}
	// 상호작용은 못하지만, 잠긴 문이 오버랩 영역에 있는 경우 힌트 출력 
	if (ClosetLockedDoor) ShowInteractionHint();
}

void UInteractionComponent::RefreshInteractionUI()
{
	TArray<AActor*> OverlappingActors;
	InteractionSphere->GetOverlappingActors(OverlappingActors);

	AActor* BestTarget = nullptr;
	float MinDistSq = MAX_FLT;

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor->GetClass()->ImplementsInterface(UInteractionBase::StaticClass()))
		{
			IInteractionBase* Interface = Cast<IInteractionBase>(Actor);
			if (Interface && Interface->GetBCanInteract())
			{
				float DistSq = FVector::DistSquared(OwnerCharacter->GetActorLocation(), Actor->GetActorLocation());
				if (DistSq < MinDistSq)
				{
					MinDistSq = DistSq;
					BestTarget = Actor;
				}
			}
		}
	}

	if (BestTarget)
	{
		IInteractionBase::Execute_ShowPressEWidget(BestTarget);
	}
}

void UInteractionComponent::ShowInteractionHint()
{
	UE_LOG(LogTemp, Warning, TEXT("ShowInteractionHint Called!"));
	if (!OwnerCharacter) return;
	if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UInteractionHintSubsystem* HintSubsystem = LP->GetSubsystem<UInteractionHintSubsystem>())
			{
				UE_LOG(LogTemp, Warning, TEXT("Subsystem Found! Showing Hint..."));
				HintSubsystem->ShowHint(2.0f);
				return;
			}
		}
	}
	UE_LOG(LogTemp, Error, TEXT("Failed to find Subsystem or PlayerController!"));
}

void UInteractionComponent::HandleDoorInteraction(AActor* DoorActor)
{
	if (!OwnerCharacter || bIsInteractingDoor) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (DoorActor == LastInteractedDoorActor && (CurrentTime - LastDoorInteractTime) < 2.5f) return;

	LastInteractedDoorActor = DoorActor;
	LastDoorInteractTime = CurrentTime;

	IInteractionBase::Execute_HidePressEWidget(DoorActor);

	OwnerCharacter->AbortAllActions();
	if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
	{
		OwnerCharacter->DisableInput(PC);
	}
	if (OwnerCharacter->GetCameraBoom())
	{
		OwnerCharacter->GetCameraBoom()->bDoCollisionTest = false;
	}
	
	if (DoorActor->IsA(ASlidingDoor::StaticClass()) || DoorActor->IsA(ADoubleDoor::StaticClass()))
	{
		bIsInteractingDoor = true;
		USoundBase* SoundToPlay = nullptr;

		if (SoundToPlay)
		{
			UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, DoorActor->GetActorLocation());
		}

		IInteractionBase::Execute_OnIneracted(DoorActor, OwnerCharacter);

		if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
		{
			OwnerCharacter->EnableInput(PC);
			PC->ResetIgnoreInputFlags();
		}

		GetWorld()->GetTimerManager().SetTimerForNextTick([this]() { bIsInteractingDoor = false; });
		return;
	}

	if (bIsInteractingDoor) return;
	bIsInteractingDoor = true;
	OwnerCharacter->bIsInteractingDoor = true;

	PendingDoorActor = DoorActor;
	CurrentInteractingItem = DoorActor;
	CurrentPickupLocation = IInteractionBase::Execute_GetInteractionTargetLocation(PendingDoorActor);

	EInteractionType Type = IInteractionBase::Execute_GetInteractionType(DoorActor);
	FVector TargetWarpLocation = FVector::ZeroVector;
	FRotator TargetWarpRotation = FRotator::ZeroRotator;
	UAnimMontage* SelectedMontage = nullptr;

	// Push/Pull 에 따른 워핑 로직 분리
	if (OwnerCharacter->MotionWarpingComp)
	{
		if (ASingleDoor* SingleDoor = Cast<ASingleDoor>(DoorActor))
		{
			if (SingleDoor->GetSingleDoorAnimationType() == ESingleDoorAnimationType::SingleDoor_Pull)
			{
				TargetWarpLocation = IInteractionBase::Execute_GetInteractionTargetLocation(DoorActor);
				FVector DirToDoor = (DoorActor->GetActorLocation() - TargetWarpLocation).GetSafeNormal2D();
				TargetWarpRotation = DirToDoor.Rotation();
				SelectedMontage = OwnerCharacter->AnimData->DoorPullOpenMontage;
			}
			else
			{
				FVector DirToHandle = (CurrentPickupLocation - OwnerCharacter->GetActorLocation()).GetSafeNormal2D();
				TargetWarpLocation = CurrentPickupLocation - (DirToHandle * 85.0f);
				TargetWarpRotation = DirToHandle.Rotation();
				SelectedMontage = OwnerCharacter->AnimData->DoorPushOpenMontage;
			}
		}
		else if (Type == EInteractionType::SafeBox)
		{
			TargetWarpLocation = CurrentPickupLocation;
			FVector DirToDoor = (DoorActor->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal2D();
			TargetWarpRotation = DirToDoor.Rotation();
			SelectedMontage = OwnerCharacter->AnimData->DoorPullOpenMontage;
		}

		TargetWarpRotation.Pitch = 0.f;
		TargetWarpRotation.Roll = 0.f;

		OwnerCharacter->MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(TEXT("DoorWarp"), TargetWarpLocation, TargetWarpRotation);

		if (SelectedMontage)
		{
			if (UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance())
			{
				AnimInst->RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
			}
			OwnerCharacter->PlayAnimMontage(SelectedMontage);
		}
	}
	IInteractionBase::Execute_OnIneracted(DoorActor, OwnerCharacter);
}

void UInteractionComponent::HandleItemInteraction(AActor* ItemActor)
{
	if (!OwnerCharacter || !ItemActor) return;
	if (OwnerCharacter->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying()) return;
	if (CurrentInteractingItem)
	{
		ConsumeInteractingItem();
	}

	// 현재 픽업한 아이템 UI를 우선적으로 숨김 
	IInteractionBase::Execute_HidePressEWidget(ItemActor);

	// 충돌을 꺼서 연속 상호작용 방지 
	ItemActor->SetActorEnableCollision(false);

	if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
	}

	if (OwnerCharacter->GetCameraBoom()) {
		OwnerCharacter->GetCameraBoom()->bDoCollisionTest = false;
	}

	CurrentInteractingItem = ItemActor;
	CurrentPickupLocation = IInteractionBase::Execute_GetInteractionTargetLocation(CurrentInteractingItem);

	if (OwnerCharacter->MotionWarpingComp)
	{
		FVector DirectionToTarget = CurrentPickupLocation - OwnerCharacter->GetActorLocation();
		DirectionToTarget.Z = 0.0f;

		if (!DirectionToTarget.IsNearlyZero())
		{
			FRotator TargetWarpRotation = DirectionToTarget.Rotation();
			OwnerCharacter->MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(TEXT("ItemWarp"), OwnerCharacter->GetActorLocation(), TargetWarpRotation);
		}
	}

	FVector LocalItemPos = OwnerCharacter->GetActorTransform().InverseTransformPosition(CurrentPickupLocation);
	UAnimMontage* MontageToPlay = (LocalItemPos.Z < -50.0f) ? OwnerCharacter->AnimData->PickupLowMontage : OwnerCharacter->AnimData->PickupHighMontage;


	if (MontageToPlay)
	{
		if (UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance())
		{
			AnimInst->RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
		}
		OwnerCharacter->PlayAnimMontage(MontageToPlay);
	}
	// 주변에 다른 아이템이 있는 경우 0.1초 뒤 UI 갱신 
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UInteractionComponent::RefreshInteractionUI);
}

void UInteractionComponent::HandleLeverInteraction(AActor* LeverActor)
{
	if (!OwnerCharacter || bIsInteractingDoor) return;

	ALever* Lever = Cast<ALever>(LeverActor);
	if (!Lever) return;

	IInteractionBase::Execute_HidePressEWidget(LeverActor);

	CurrentInteractingItem = LeverActor;
	bIsInteractingDoor = true;

	// ESC 허용을 위해 이동/회전만 무시
	if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
	}

	CurrentPickupLocation = Lever->PickUpPoint->GetComponentLocation();

	if (OwnerCharacter->MotionWarpingComp)
	{
		FVector LeverForward = Lever->GetActorRightVector();
		FVector TargetWarpLocation = Lever->GetActorLocation() + (LeverForward * 55.0f);
		TargetWarpLocation.Z = OwnerCharacter->GetActorLocation().Z;
		FRotator TargetWarpRotation = (-LeverForward).Rotation();

		OwnerCharacter->MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(TEXT("LeverWarp"), TargetWarpLocation, TargetWarpRotation);
	}

	if (OwnerCharacter->AnimData && OwnerCharacter->AnimData->LeverMontage)
	{
		if (UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance())
		{
			AnimInst->RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
		}
		OwnerCharacter->PlayAnimMontage(OwnerCharacter->AnimData->LeverMontage);
	}
}

void UInteractionComponent::AttachInteractingItem()
{
	if (CurrentInteractingItem && OwnerCharacter)
	{
		CurrentInteractingItem->SetActorEnableCollision(false);
		CurrentInteractingItem->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ItemSocket"));
		CurrentInteractingItem->SetActorScale3D(FVector(0.8f));
	}
}

void UInteractionComponent::ConsumeInteractingItem()
{
	if (CurrentInteractingItem && OwnerCharacter)
	{
		IInteractionBase::Execute_OnIneracted(CurrentInteractingItem, OwnerCharacter);
		CurrentInteractingItem = nullptr;
	}
}

void UInteractionComponent::OnInteractableBeganOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == OwnerCharacter) return;

	if (OtherActor->GetClass()->ImplementsInterface(UInteractionBase::StaticClass()))
	{
		IInteractionBase* InteractInterface = Cast<IInteractionBase>(OtherActor);

		if (InteractInterface && InteractInterface->GetBCanInteract())
		{
			IInteractionBase::Execute_ShowPressEWidget(OtherActor);
		}
	}
}

void UInteractionComponent::OnInteractableEndedOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || OtherActor == OwnerCharacter) return;

	if (OtherActor->GetClass()->ImplementsInterface(UInteractionBase::StaticClass()))
	{
		IInteractionBase::Execute_HidePressEWidget(OtherActor);

		// 아이템이 사라졌으므로, 주변에 남은 다른 아이템이 있는지 체크 후 UI 띄움.
		RefreshInteractionUI();
	}
}

	
void UInteractionComponent::TriggerInteraction()
{
	if (CurrentInteractingItem && IsValid(CurrentInteractingItem))
	{
		IInteractionBase::Execute_OnIneracted(CurrentInteractingItem, OwnerCharacter);
	}
}

void UInteractionComponent::EndInteraction()
{
	bIsInteractingDoor = false;
	if (OwnerCharacter) OwnerCharacter->bIsInteractingDoor = false;
	CurrentInteractingItem = nullptr;

	if (OwnerCharacter)
	{
		OwnerCharacter->DestroyHealItemVisual();

		if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
		{
			OwnerCharacter->EnableInput(PC);
			PC->ResetIgnoreInputFlags();
		}
		if (OwnerCharacter->GetCameraBoom())
		{
			OwnerCharacter->GetCameraBoom()->bDoCollisionTest = true;
		}
	}
}

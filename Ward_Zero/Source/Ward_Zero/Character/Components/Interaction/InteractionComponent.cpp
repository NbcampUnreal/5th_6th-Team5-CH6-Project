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
#include "GameFramework/SpringArmComponent.h"
#include "UI_KWJ/InteractionHint/InteractionHintSubsystem.h"

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

	TArray<AActor*> OverlappingActors;
	InteractionSphere->GetOverlappingActors(OverlappingActors);

	AActor* ClosestInteractable = nullptr;
	float MinDistanceSquared = MAX_FLT;
	FVector PlayerLocation = OwnerCharacter->GetActorLocation();

	for (AActor* Actor : OverlappingActors)
	{
		if (!Actor || Actor == OwnerCharacter) continue;

		bool bIsValidInteractable = false;

		if (Actor->GetClass()->ImplementsInterface(UInteractionBase::StaticClass()))
		{
			IInteractionBase* InteractInterface = Cast<IInteractionBase>(Actor);
			if (InteractInterface && InteractInterface->GetBCanInteract())
			{
				bIsValidInteractable = true;
			}
		}

		if (bIsValidInteractable)
		{
			float DistSq = FVector::DistSquared(PlayerLocation, Actor->GetActorLocation());
			if (DistSq < MinDistanceSquared)
			{
				MinDistanceSquared = DistSq;
				ClosestInteractable = Actor;
			}
		}
	}

	if (!ClosestInteractable) return;

	if (ClosestInteractable->GetClass()->ImplementsInterface(UInteractionBase::StaticClass()))
	{
		IInteractionBase::Execute_HidePressEWidget(ClosestInteractable);
		EInteractionType Type = IInteractionBase::Execute_GetInteractionType(ClosestInteractable);

		if (Type == EInteractionType::Door) HandleDoorInteraction(ClosestInteractable);
		else if (Type == EInteractionType::Ammo || Type == EInteractionType::Heal || Type == EInteractionType::Key)
			HandleItemInteraction(ClosestInteractable);
		else if (Type == EInteractionType::Lever)
			HandleLeverInteraction(ClosestInteractable);
		else
		{
			IInteractionBase::Execute_OnIneracted(ClosestInteractable, OwnerCharacter);
		}
	}
}

void UInteractionComponent::HandleDoorInteraction(AActor* DoorActor)
{
	if (!OwnerCharacter) return;

	bool bIsSameDoor = (DoorActor == LastInteractedDoorActor);
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (bIsSameDoor && (CurrentTime - LastDoorInteractTime) < 2.5f) return;

	LastInteractedDoorActor = DoorActor;
	LastDoorInteractTime = CurrentTime;

	ASingleDoor* SingleDoor = Cast<ASingleDoor>(DoorActor);
	ASafeActor* SafeDoor = Cast<ASafeActor>(DoorActor);

	if (!SingleDoor && !SafeDoor)
	{
		IInteractionBase::Execute_OnIneracted(DoorActor, OwnerCharacter);
		return;
	}

	if (bIsInteractingDoor) return;
	bIsInteractingDoor = true;
	OwnerCharacter->bIsInteractingDoor = true;

	PendingDoorActor = DoorActor;
	CurrentInteractingItem = DoorActor;
	CurrentPickupLocation = IInteractionBase::Execute_GetInteractionTargetLocation(PendingDoorActor);

	if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
	}

	// Push/Pull 에 따른 워핑 로직 분리
	if (OwnerCharacter->MotionWarpingComp)
	{
		FVector TargetWarpLocation;
		FRotator TargetWarpRotation;

		UAnimMontage* SelectedMontage = nullptr;
		if (SingleDoor)
		{
			if (SingleDoor->GetSingleDoorAnimationType() == ESingleDoorAnimationType::SingleDoor_Pull)
			{
				TargetWarpLocation = SingleDoor->PullPoint->GetComponentLocation();
				TargetWarpRotation = SingleDoor->PullPoint->GetComponentRotation();
				SelectedMontage = OwnerCharacter->AnimData->DoorPullOpenMontage;
			}
			else
			{
				FVector HandleLocation = CurrentPickupLocation;
				FVector CharacterLocation = OwnerCharacter->GetActorLocation();

				// 캐릭터에서 손잡이를 바라보는 방향 계산
				FVector DirToHandle = (HandleLocation - CharacterLocation).GetSafeNormal2D();

				// 손잡이 정면에서 85유닛 떨어진 곳을 타겟으로 설정
				TargetWarpLocation = HandleLocation - (DirToHandle * 85.0f);
				TargetWarpRotation = DirToHandle.Rotation();
				SelectedMontage = OwnerCharacter->AnimData->DoorPushOpenMontage;
			}
		}
		// 금고 - Pull
		else if (SafeDoor)
		{
			// 금고에 설정된 타겟 위치(Pull Point) 사용
			TargetWarpLocation = CurrentPickupLocation;
			TargetWarpRotation = (OwnerCharacter->GetActorLocation() - DoorActor->GetActorLocation()).Rotation();
			SelectedMontage = OwnerCharacter->AnimData->DoorPullOpenMontage;
		}

		TargetWarpRotation.Pitch = 0.f;
		TargetWarpRotation.Roll = 0.f;

		OwnerCharacter->MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(TEXT("DoorWarp"), TargetWarpLocation, TargetWarpRotation);

		if (SelectedMontage)
		{
			OwnerCharacter->PlayAnimMontage(SelectedMontage);
		}
	}
	IInteractionBase::Execute_OnIneracted(DoorActor, OwnerCharacter);
}

void UInteractionComponent::HandleItemInteraction(AActor* ItemActor)
{
	if (!OwnerCharacter || !ItemActor) return;
	if (OwnerCharacter->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying() || CurrentInteractingItem) return;
	if (CurrentInteractingItem) ConsumeInteractingItem();

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
}

void UInteractionComponent::HandleLeverInteraction(AActor* LeverActor)
{
	if (!OwnerCharacter || bIsInteractingDoor) return;

	ALever* Lever = Cast<ALever>(LeverActor);
	if (!Lever) return;

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
		CurrentInteractingItem->Destroy(); // 월드에서 진짜 액터 삭제
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
			PC->SetIgnoreMoveInput(false);
			PC->SetIgnoreLookInput(false);
			OwnerCharacter->EnableInput(PC);
		}
		if (OwnerCharacter->GetCameraBoom())
		{
			OwnerCharacter->GetCameraBoom()->bDoCollisionTest = true;
		}
	}
}

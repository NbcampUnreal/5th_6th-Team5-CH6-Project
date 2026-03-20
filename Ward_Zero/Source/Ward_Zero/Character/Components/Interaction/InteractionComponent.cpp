#include "Character/Components/Interaction/InteractionComponent.h"
#include "Components/BoxComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Gimmic_CY/Base/InteractionBase.h"
#include "UI_KWJ/Save/SaveSubsystem.h"
#include "MotionWarpingComponent.h"
#include "Character/Data/AnimData/CharacterAnimData.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Gimmic_CY/Lever.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteractionComponent::Initialize(APrototypeCharacter* InCharacter, UBoxComponent* InBox)
{
	OwnerCharacter = InCharacter;
	InteractableBox = InBox;
}

void UInteractionComponent::TryInteract()
{
	if (!OwnerCharacter || !InteractableBox) return;

	TArray<AActor*> OverlappingActors;
	InteractableBox->GetOverlappingActors(OverlappingActors);

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
		else if (Actor->ActorHasTag(TEXT("SavePoint")))
		{
			bIsValidInteractable = true;
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
		EInteractionType Type = IInteractionBase::Execute_GetInteractionType(ClosestInteractable);

		if (Type == EInteractionType::Door) HandleDoorInteraction(ClosestInteractable);
		else if (Type == EInteractionType::Ammo || Type == EInteractionType::Heal || Type == EInteractionType::Key) HandleItemInteraction(ClosestInteractable);
	}
	else if (ClosestInteractable->ActorHasTag(TEXT("SavePoint")))
	{
		if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
		{
			if (USaveSubsystem* SaveSub = PC->GetLocalPlayer()->GetSubsystem<USaveSubsystem>())
			{
				SaveSub->ShowSaveUI();
			}
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

	if (bIsInteractingDoor) return;
	bIsInteractingDoor = true;

	PendingDoorActor = DoorActor;
	CurrentPickupLocation = IInteractionBase::Execute_GetInteractionTargetLocation(PendingDoorActor);

	OwnerCharacter->GetCapsuleComponent()->IgnoreActorWhenMoving(PendingDoorActor, true);

	if (OwnerCharacter->MotionWarpingComp)
	{
		if (bIsSameDoor)
		{
			OwnerCharacter->MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(TEXT("DoorWarp"), OwnerCharacter->GetActorLocation(), OwnerCharacter->GetActorRotation());
		}
		else
		{
			FVector DirectionToPlayer = (OwnerCharacter->GetActorLocation() - CurrentPickupLocation).GetSafeNormal2D();
			FVector TargetWarpLocation = CurrentPickupLocation + (DirectionToPlayer * 65.0f);
			FRotator TargetWarpRotation = (-DirectionToPlayer).Rotation();

			OwnerCharacter->MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(TEXT("DoorWarp"), TargetWarpLocation, TargetWarpRotation);
		}
	}

	if (OwnerCharacter->AnimData && OwnerCharacter->AnimData->OpenDoorMontage)
	{
		float AnimDuration = OwnerCharacter->PlayAnimMontage(OwnerCharacter->AnimData->OpenDoorMontage);
		AActor* SafeDoorActor = PendingDoorActor;
		APrototypeCharacter* SafeCharacter = OwnerCharacter; // 람다 캡처용

		FTimerHandle DoorTimer;
		GetWorld()->GetTimerManager().SetTimer(DoorTimer, [this, SafeCharacter, SafeDoorActor]()
			{
				bIsInteractingDoor = false;
				if (SafeCharacter)
				{
					if (APlayerController* PC = Cast<APlayerController>(SafeCharacter->GetController())) SafeCharacter->EnableInput(PC);

					if (SafeDoorActor)
					{
						FTimerHandle CollisionRestoreTimer;
						SafeCharacter->GetWorldTimerManager().SetTimer(CollisionRestoreTimer, FTimerDelegate::CreateLambda([SafeCharacter, SafeDoorActor]()
							{
								if (IsValid(SafeCharacter) && IsValid(SafeDoorActor))
									SafeCharacter->GetCapsuleComponent()->IgnoreActorWhenMoving(SafeDoorActor, false);
							}), 1.5f, false);
					}
				}
			}, AnimDuration, false);
	}

	IInteractionBase::Execute_OnIneracted(DoorActor, OwnerCharacter);
}

void UInteractionComponent::HandleItemInteraction(AActor* ItemActor)
{
	if (!OwnerCharacter) return;

	if (CurrentInteractingItem) ConsumeInteractingItem();

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

	if (MontageToPlay) OwnerCharacter->PlayAnimMontage(MontageToPlay);
}

void APrototypeCharacter::HandleLeverInteraction(AActor* LeverActor)
{
	if (bIsInteractingDoor) return;
	bIsInteractingDoor = true;

	ALever* Lever = Cast<ALever>(LeverActor);
	if (!Lever)
	{
		bIsInteractingDoor = false;
		return;
	}

	// 워핑 및 IK를 위한 타겟 위치(PickUpPoint) 갱신
	CurrentPickupLocation = IInteractionBase::Execute_GetInteractionTargetLocation(Lever);

	// 재생할 몽타주 결정 
	UAnimMontage* MontageToPlay = AnimData->LeverMontage;

	if (MontageToPlay)
	{
		// 몽타주 재생 및 전체 재생 시간 확보
		float AnimDuration = PlayAnimMontage(MontageToPlay);

		// 실제 레버 작동 트리거 (애니메이션의 약 50% 시점에 실행)
		FTimerHandle LeverTriggerTimer;
		GetWorldTimerManager().SetTimer(LeverTriggerTimer, [Lever, this]()
			{
				if (IsValid(Lever))
				{
					IInteractionBase::Execute_OnIneracted(Lever, this);
				}
			}, AnimDuration * 0.5f, false);

		// 상호작용 상태 해제 타이머
		FTimerHandle EndTimer;
		GetWorldTimerManager().SetTimer(EndTimer, [this]()
			{
				bIsInteractingDoor = false;
			}, AnimDuration, false);
	}
	else
	{
		// 몽타주가 없을 경우 즉시 실행하고 상태 해제
		IInteractionBase::Execute_OnIneracted(Lever, this);
		bIsInteractingDoor = false;
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

	
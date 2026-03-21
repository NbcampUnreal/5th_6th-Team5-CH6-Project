#include "Character/Components/Interaction/InteractionComponent.h"
#include "Components/BoxComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Gimmic_CY/Interface/InteractionBase.h"
#include "UI_KWJ/Save/SaveSubsystem.h"
#include "MotionWarpingComponent.h"
#include "Character/Data/AnimData/CharacterAnimData.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Gimmic_CY/Object/Lever/Lever.h"
#include "Gimmic_CY/Items/ItemBase.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteractionComponent::Initialize(APrototypeCharacter* InCharacter, UBoxComponent* InBox)
{
	OwnerCharacter = InCharacter;
	InteractableBox = InBox;

	if (InteractableBox)
	{
		InteractableBox->OnComponentBeginOverlap.AddDynamic(this, &UInteractionComponent::OnInteractableBeganOverlap);
		InteractableBox->OnComponentEndOverlap.AddDynamic(this, &UInteractionComponent::OnInteractableEndedOverlap);
	}
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
		else if (Type == EInteractionType::Ammo || Type == EInteractionType::Heal || Type == EInteractionType::Key) 
			HandleItemInteraction(ClosestInteractable);
		else if (Type == EInteractionType::Lever) 
			HandleLeverInteraction(ClosestInteractable); 
		else if (Type == EInteractionType::Save)
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

void UInteractionComponent::HandleLeverInteraction(AActor* LeverActor)
{
	if (!OwnerCharacter || bIsInteractingDoor) return;

	ALever* Lever = Cast<ALever>(LeverActor);
	if (!Lever) return;

	CurrentInteractingItem = LeverActor;
	bIsInteractingDoor = true;

	// IK 타겟 위치 저장 (레버의 CollisionBox 위치)
	CurrentPickupLocation = IInteractionBase::Execute_GetIKTargetLocation(Lever);

	// 모션 워핑
	if (OwnerCharacter->MotionWarpingComp)
	{
		FVector LeverLoc = Lever->GetActorLocation();
		FVector LeverForward = Lever->GetActorForwardVector();

		// 레버 정면 65유닛 지점 계산
		FVector TargetWarpLocation = LeverLoc + (LeverForward * 65.0f);
		// 캐릭터가 바닥에 발을 붙이도록 Z축은 현재 캐릭터 높이 유지
		TargetWarpLocation.Z = OwnerCharacter->GetActorLocation().Z;

		// 레버를 마주보도록 회전값 계산
		FRotator TargetWarpRotation = (-LeverForward).Rotation();
		TargetWarpRotation.Pitch = 0.0f;
		TargetWarpRotation.Roll = 0.0f;

		OwnerCharacter->MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(
			TEXT("LeverWarp"),
			TargetWarpLocation,
			TargetWarpRotation
		);
	}

	// 몽타주 재생 
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
		CurrentInteractingItem->Destroy();
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
	CurrentInteractingItem = nullptr;
}

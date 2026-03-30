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

	InteractionSphere->SetSphereRadius(120.0f);
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
	AActor* ClosestLockedOther = nullptr; // SigleDoor 외에 Lever, SafeBox, Button 등 잠긴 오브젝트 저장 

	float MinInteractDistSq = MAX_FLT;
	float MinLockedDistSq = MAX_FLT;
	float MinLockedOtherDistSq = MAX_FLT;

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
			// Door / SingleDoor: 잠긴 문 전용 변수에 저장 (몽타주 재생)
			if (Type == EInteractionType::Door || Type == EInteractionType::SingleDoor)
			{
				if (DistSq < MinLockedDistSq)
				{
					MinLockedDistSq = DistSq;
					ClosetLockedDoor = Actor;
				}
			}
			// Lever, SafeBox, Button 등 잠긴 오브젝트 전용 변수에 저장 
			else if (Type == EInteractionType::Lever
				|| Type == EInteractionType::SafeBox
				|| Type == EInteractionType::Button)
			{
				if (DistSq < MinLockedOtherDistSq)
				{
					MinLockedOtherDistSq = DistSq;
					ClosestLockedOther = Actor;
				}
			}
		}
	}

	// 상호작용 실행 
	if (ClosestInteractable)
	{
		EInteractionType Type = IInteractionBase::Execute_GetInteractionType(ClosestInteractable);

		if (Type == EInteractionType::Door || Type == EInteractionType::SingleDoor || Type == EInteractionType::SafeBox)
			HandleDoorInteraction(ClosestInteractable);
		else if (Type == EInteractionType::Ammo || Type == EInteractionType::Heal || Type == EInteractionType::Key)
			HandleItemInteraction(ClosestInteractable);
		else if (Type == EInteractionType::Lever)
			HandleLeverInteraction(ClosestInteractable);
		else if (Type == EInteractionType::Button)      
			HandleButtonInteraction(ClosestInteractable);
		else
		{
			/*IInteractionBase::Execute_HidePressEWidget(ClosestInteractable);*/
			IInteractionBase::Execute_OnIneracted(ClosestInteractable, OwnerCharacter);
		}
		return;
	}
	// 상호작용은 못하지만, 잠긴 문이 오버랩 영역에 있는 경우 힌트 출력 
	if (ClosetLockedDoor)
	{
		ShowInteractionHint(TEXT("문이 굳게 잠겨 있다. 열쇠가 필요할 것 같다."), 3.0f);
		PlayLockedDoorMontage(ClosetLockedDoor);
	}
	else if (ClosestLockedOther)
	{
		EInteractionType LockedType = IInteractionBase::Execute_GetInteractionType(ClosestLockedOther);
		if (LockedType == EInteractionType::Lever)
			ShowInteractionHint(TEXT("레버가 움직이지 않는다."), 3.0f);
		else if (LockedType == EInteractionType::SafeBox)
			ShowInteractionHint(TEXT("금고가 잠겨 있다."), 3.0f);
		else if (LockedType == EInteractionType::Button)
			ShowInteractionHint(TEXT("버튼이 반응하지 않는다."), 3.0f);
	}
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

void UInteractionComponent::ShowInteractionHint(FString Message, float Duration)
{
	if (!OwnerCharacter) return;
	if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UInteractionHintSubsystem* HintSubsystem = LP->GetSubsystem<UInteractionHintSubsystem>())
			{
				HintSubsystem->ShowHintWithText(FText::FromString(Message), Duration);
				return;
			}
		}
	}
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
	ASingleDoor* SingleDoor = Cast<ASingleDoor>(DoorActor);
	if (OwnerCharacter->MotionWarpingComp)
	{
		if (SingleDoor)
		{
			if (SingleDoor->GetSingleDoorAnimationType() == ESingleDoorAnimationType::SingleDoor_Pull)
			{
				USceneComponent* PullPoint = nullptr;
				TInlineComponentArray<USceneComponent*> SceneComps;
				DoorActor->GetComponents<USceneComponent>(SceneComps);
				for (USceneComponent* Comp : SceneComps)
				{
					if (Comp->GetFName() == FName(TEXT("PullPoint")))
					{
						PullPoint = Comp;
						break;
					}
				}

				// HandleSocket 위치 가져오기
				FVector HandleLocation = CurrentPickupLocation; // 폴백
				if (UStaticMeshComponent* DoorMesh = DoorActor->FindComponentByClass<UStaticMeshComponent>())
				{
					if (DoorMesh->DoesSocketExist(TEXT("HandleSocket")))
					{
						HandleLocation = DoorMesh->GetSocketLocation(TEXT("HandleSocket"));
					}
				}

				// HandleSocket 위치를 IK 타겟으로도 업데이트
				CurrentPickupLocation = HandleLocation;

				if (PullPoint)
				{
					TargetWarpLocation = PullPoint->GetComponentLocation();

					// PullPoint → HandleSocket 방향으로 캐릭터 회전 계산
					FVector DirToHandle = (HandleLocation - TargetWarpLocation).GetSafeNormal2D();
					if (DirToHandle.IsNearlyZero())
					{
						DirToHandle = (DoorActor->GetActorLocation() - TargetWarpLocation).GetSafeNormal2D();
					}
					TargetWarpRotation = DirToHandle.Rotation();
				}
				else
				{
					TargetWarpLocation = HandleLocation;
					FVector DirToDoor = (DoorActor->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal2D();
					TargetWarpRotation = DirToDoor.Rotation();
				}

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
	if (!SingleDoor || SingleDoor->GetSingleDoorAnimationType() != ESingleDoorAnimationType::SingleDoor_Pull)
	{
		IInteractionBase::Execute_OnIneracted(DoorActor, OwnerCharacter);
	}
}

void UInteractionComponent::HandleItemInteraction(AActor* ItemActor)
{
	if (!OwnerCharacter || !ItemActor) return;

	UAnimInstance* AI = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (AI && AI->IsAnyMontagePlaying()) return;

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

void UInteractionComponent::HandleButtonInteraction(AActor* ButtonActor)
{
	if (!OwnerCharacter || !ButtonActor) return;
	if (!OwnerCharacter->AnimData || !OwnerCharacter->AnimData->ButtonPressMontage) return;

	if (UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance())
	{
		if (AnimInst->IsAnyMontagePlaying()) return;
	}

	IInteractionBase::Execute_HidePressEWidget(ButtonActor);

	// FingerSocket 위치를 IK 타겟으로 사용
	FVector HandleLocation = ButtonActor->GetActorLocation();

	if (UStaticMeshComponent* ButtonMesh = ButtonActor->FindComponentByClass<UStaticMeshComponent>())
	{
		if (ButtonMesh->DoesSocketExist(TEXT("FingerSocket")))
		{
			HandleLocation = ButtonMesh->GetSocketLocation(TEXT("FingerSocket"));
		}
	}
	CurrentPickupLocation = HandleLocation;
	CurrentInteractingItem = ButtonActor;

	if (OwnerCharacter->MotionWarpingComp)
	{
		// -Y가 플레이어가 서야 할 정면 방향입니다.
		FVector ButtonFacing = -ButtonActor->GetActorRightVector();
		ButtonFacing.Z = 0.f;
		ButtonFacing.Normalize();

		// 정면 방향(ButtonFacing)의 반대인 +Y를 바라보게 설정 
		FRotator TargetRot = (-ButtonFacing).Rotation();
		TargetRot.Pitch = 0.f;
		TargetRot.Roll = 0.f;

		// 소켓(HandleLocation) 위치에서 버튼의 정면(-Y) 방향으로 70.0f만큼 떨어진 지점
		FVector TargetWarpLocation = HandleLocation + (ButtonFacing * 70.0f);
		TargetWarpLocation.Z = OwnerCharacter->GetActorLocation().Z;

		DrawDebugSphere(GetWorld(), TargetWarpLocation, 10.f, 12, FColor::Red, false, 2.0f);

		OwnerCharacter->MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(
			TEXT("ButtonWarp"),
			TargetWarpLocation,
			TargetRot
		);
	}

	if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
	}

	if (UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance())
		AnimInst->RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;

	OwnerCharacter->PlayAnimMontage(OwnerCharacter->AnimData->ButtonPressMontage);
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


void UInteractionComponent::PlayLockedDoorMontage(AActor* DoorActor)
{
	if (!OwnerCharacter || !OwnerCharacter->AnimData) return;
	if (!OwnerCharacter->AnimData->LockedDoorMontage) return;

	// SingleDoor일 때만 유효
	ASingleDoor* SingleDoor = Cast<ASingleDoor>(DoorActor);
	if (!SingleDoor) return;

	if (UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance())
	{
		if (AnimInst->IsAnyMontagePlaying()) return;
	}

	if (!OwnerCharacter->MotionWarpingComp) return;

	// HandleSocket 위치 (IK 타겟)
	FVector HandleLocation = DoorActor->GetActorLocation(); // 폴백
	if (UStaticMeshComponent* DoorMesh = DoorActor->FindComponentByClass<UStaticMeshComponent>())
	{
		if (DoorMesh->DoesSocketExist(TEXT("HandleSocket")))
		{
			HandleLocation = DoorMesh->GetSocketLocation(TEXT("HandleSocket"));
		}
	}

	// IK 타겟으로 등록 
	CurrentPickupLocation = HandleLocation;
	CurrentInteractingItem = DoorActor;
	CurrentPickupLocation += FVector(-2.27f, -0.79f, -30.f);

	// PullPoint 위치 (캐릭터 이동 워프 타겟)
	FVector WarpLocation = OwnerCharacter->GetActorLocation(); // 폴백
	USceneComponent* PullPoint = nullptr;
	TInlineComponentArray<USceneComponent*> SceneComps;
	DoorActor->GetComponents<USceneComponent>(SceneComps);
	for (USceneComponent* Comp : SceneComps)
	{
		if (Comp->GetFName() == FName(TEXT("PullPoint")))
		{
			PullPoint = Comp;
			break;
		}
	}
	if (PullPoint)
	{
		WarpLocation = PullPoint->GetComponentLocation();
		WarpLocation.Z = OwnerCharacter->GetActorLocation().Z;
	}

	// 회전 워핑 PullPoint -> HandleSocket 방향
	FVector DirToHandle = (HandleLocation - WarpLocation).GetSafeNormal2D();
	if (DirToHandle.IsNearlyZero())
		DirToHandle = (DoorActor->GetActorLocation() - WarpLocation).GetSafeNormal2D();
	FRotator TargetRot = DirToHandle.Rotation();
	TargetRot.Pitch = 0.f;
	TargetRot.Roll = 0.f;

	// 입력 잠금
	if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
	}
	bIsInteractingDoor = true;                         
	OwnerCharacter->bIsInteractingDoor = true;

	StartCameraAlign(DoorActor, [this, WarpLocation, TargetRot]()
		{
			if (!OwnerCharacter || !OwnerCharacter->MotionWarpingComp) return;

			OwnerCharacter->MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(
				TEXT("LockedDoorWarp"), WarpLocation, TargetRot);

			if (UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance())
				AnimInst->RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;

			OwnerCharacter->PlayAnimMontage(OwnerCharacter->AnimData->LockedDoorMontage);
		});
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

void UInteractionComponent::AlignCharacterToPullPoint(AActor* Interactable)
{
	if (!Interactable) return;

	APrototypeCharacter* Character = Cast<APrototypeCharacter>(GetOwner());
	if (!Character) return;

	// PullPoint 컴포넌트 탐색
	USceneComponent* PullPoint = nullptr;
	TInlineComponentArray<USceneComponent*> SceneComps;
	Interactable->GetComponents<USceneComponent>(SceneComps);

	for (USceneComponent* Comp : SceneComps)
	{
		if (Comp->GetFName() == FName(TEXT("PullPoint")))
		{
			PullPoint = Comp;
			break;
		}
	}

	// PullPoint 없으면 스킵 (Push 타입 등)
	if (!PullPoint) return;

	// 위치 정렬
	const FVector PullLoc = PullPoint->GetComponentLocation();
	const FVector TargetLoc = FVector(PullLoc.X, PullLoc.Y, Character->GetActorLocation().Z);

	if (FVector::Dist2D(Character->GetActorLocation(), TargetLoc) > 30.f)
	{
		Character->SetActorLocation(TargetLoc, false, nullptr, ETeleportType::TeleportPhysics);
	}

	// 회전 정렬 (컨트롤러 먼저, 액터 나중)
	const float TargetYaw = PullPoint->GetComponentRotation().Yaw;

	if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
	{
		FRotator CtrlRot = PC->GetControlRotation();
		CtrlRot.Yaw = TargetYaw;
		PC->SetControlRotation(CtrlRot);
	}

	Character->SetActorRotation(FRotator(0.f, TargetYaw, 0.f));
}

void UInteractionComponent::StartCameraAlign(AActor* Target, TFunction<void()> OnComplete)
{
	PendingAlignTarget = Target;
	CameraAlignElapsed = 0.f;

	// 즉시 LookInput 차단
	if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
		PC->SetIgnoreLookInput(true);

	GetWorld()->GetTimerManager().SetTimer(CameraAlignTimer,
		FTimerDelegate::CreateLambda([this, OnComplete]()
			{
				TickCameraAlign(OnComplete);
			}), 0.016f, true);
}

void UInteractionComponent::TickCameraAlign(TFunction<void()> OnComplete)
{
	CameraAlignElapsed += 0.016f;

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC || !PendingAlignTarget)
	{
		GetWorld()->GetTimerManager().ClearTimer(CameraAlignTimer);
		OnComplete();
		return;
	}

	FVector Dir = (PendingAlignTarget->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal2D();
	FRotator TargetRot = Dir.Rotation();
	TargetRot.Pitch = PC->GetControlRotation().Pitch; // Pitch는 유지

	FRotator Current = PC->GetControlRotation();
	FRotator NewRot = FMath::RInterpTo(Current, TargetRot, 0.016f, 10.0f);
	PC->SetControlRotation(NewRot);

	float YawDelta = FMath::Abs(FRotator::NormalizeAxis(TargetRot.Yaw - NewRot.Yaw));

	// 정렬 완료 OR 타임아웃(0.4초)
	if (YawDelta < 3.0f || CameraAlignElapsed >= 0.4f)
	{
		GetWorld()->GetTimerManager().ClearTimer(CameraAlignTimer);
		PendingAlignTarget = nullptr;
		OnComplete(); // 워핑 + 몽타주 실행
	}
}

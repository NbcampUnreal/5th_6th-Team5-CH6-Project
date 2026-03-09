// InteractionComponent.cpp

#include "Character/Components/Interaction/InteractionComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Data/AnimData/CharacterAnimData.h"
#include "Objects/Interface/Interact.h"
#include "Gimmic_CY/InteractionBase.h"
#include "UI_KWJ/Save/SavePointComponent.h"
#include "UI_KWJ/Save/SaveSubsystem.h"
#include "UI_KWJ/GameClear/GameClearComponent.h"
#include "UI_KWJ/GameClear/GameClearSubsystem.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Ward_Zero.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	FocusedActor = nullptr;
	PromptWidgetComp = nullptr;
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// 프롬프트 위젯 컴포넌트 동적 생성
	if (PromptWidgetClass)
	{
		PromptWidgetComp = NewObject<UWidgetComponent>(GetOwner(), TEXT("InteractPrompt"));
		if (PromptWidgetComp)
		{
			PromptWidgetComp->RegisterComponent();
			PromptWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
			PromptWidgetComp->SetWidgetClass(PromptWidgetClass);
			PromptWidgetComp->SetDrawAtDesiredSize(true);
			PromptWidgetComp->SetVisibility(false);
			PromptWidgetComp->AttachToComponent(
				GetOwner()->GetRootComponent(),
				FAttachmentTransformRules::KeepRelativeTransform
			);
		}
	}
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	PerformTrace();
	UpdatePromptLocation();
}

// ══════════════════════════════════════════
//  라인 트레이스
// ══════════════════════════════════════════

void UInteractionComponent::PerformTrace()
{
	APrototypeCharacter* Player = Cast<APrototypeCharacter>(GetOwner());
	if (!Player) return;

	// 카메라 기준 트레이스
	UCameraComponent* Camera = Player->FindComponentByClass<UCameraComponent>();
	if (!Camera) return;

	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + (Camera->GetForwardVector() * InteractionRange);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);

	AActor* HitActor = nullptr;

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, TraceChannel, Params))
	{
		AActor* Actor = Hit.GetActor();
		if (Actor && IsInteractable(Actor))
		{
			HitActor = Actor;
		}
	}

	SetFocusedActor(HitActor);
}

// ══════════════════════════════════════════
//  포커스 관리
// ══════════════════════════════════════════

void UInteractionComponent::SetFocusedActor(AActor* NewActor)
{
	if (FocusedActor == NewActor) return;

	FocusedActor = NewActor;

	if (PromptWidgetComp)
	{
		PromptWidgetComp->SetVisibility(FocusedActor != nullptr);
	}
}

void UInteractionComponent::UpdatePromptLocation()
{
	if (!PromptWidgetComp || !FocusedActor) return;

	// 액터 바운딩 박스 상단 + 오프셋에 프롬프트 배치
	FVector Origin, Extent;
	FocusedActor->GetActorBounds(false, Origin, Extent);

	FVector PromptLocation = Origin + FVector(0.0f, 0.0f, Extent.Z + PromptHeightOffset);
	PromptWidgetComp->SetWorldLocation(PromptLocation);
}

// ══════════════════════════════════════════
//  상호작용 체크 & 실행
// ══════════════════════════════════════════

bool UInteractionComponent::IsInteractable(AActor* Actor) const
{
	if (!Actor) return false;

	// SavePoint 태그 또는 SavePointComponent
	if (Actor->ActorHasTag(TEXT("SavePoint")) ||
		Actor->FindComponentByClass<USavePointComponent>())
	{
		return true;
	}

	// GameClear 태그 또는 GameClearComponent
	if (Actor->ActorHasTag(TEXT("GameClear")) ||
		Actor->FindComponentByClass<UGameClearComponent>())
	{
		return true;
	}

	// IInteract (BaseObject 계열: 서류 등)
	if (Actor->GetClass()->ImplementsInterface(UInteract::StaticClass()))
	{
		return true;
	}

	// IInteractionBase (Gimmic_CY 계열: 문 등)
	if (Actor->GetClass()->ImplementsInterface(UInteractionBase::StaticClass()))
	{
		//IInteractionBase* Interactable = Cast<IInteractionBase>(Actor);
		//if (Interactable && Interactable->CanBeInteracted())
		//{
		//	return true;
		//}
		if (IInteractionBase::Execute_CanBeInteracted(Actor))
		{
			return true;
		}
	}

	return false;
}

void UInteractionComponent::ExecuteInteraction(AActor* Actor)
{
	if (!Actor) return;

	APrototypeCharacter* Player = Cast<APrototypeCharacter>(GetOwner());
	if (!Player) return;

	// SavePoint (태그 또는 컴포넌트)
	USavePointComponent* SaveComp = Actor->FindComponentByClass<USavePointComponent>();
	if (SaveComp)
	{
		SaveComp->ActivateSavePoint(Player);
		return;
	}
	if (Actor->ActorHasTag(TEXT("SavePoint")))
	{
		APlayerController* PC = Cast<APlayerController>(Player->GetController());
		if (PC)
		{
			ULocalPlayer* LP = PC->GetLocalPlayer();
			if (LP)
			{
				USaveSubsystem* SaveSub = LP->GetSubsystem<USaveSubsystem>();
				if (SaveSub) SaveSub->ShowSaveUI();
			}
		}
		return;
	}

	// GameClear (태그 또는 컴포넌트)
	UGameClearComponent* ClearComp = Actor->FindComponentByClass<UGameClearComponent>();
	if (ClearComp)
	{
		ClearComp->ActivateGameClear(Player);
		return;
	}
	if (Actor->ActorHasTag(TEXT("GameClear")))
	{
		APlayerController* PC = Cast<APlayerController>(Player->GetController());
		if (PC)
		{
			ULocalPlayer* LP = PC->GetLocalPlayer();
			if (LP)
			{
				UGameClearSubsystem* ClearSys = LP->GetSubsystem<UGameClearSubsystem>();
				if (ClearSys) ClearSys->ShowGameClear(600.f);
			}
		}
		return;
	}

	// IInteract (서류 등)
	if (Actor->GetClass()->ImplementsInterface(UInteract::StaticClass()))
	{
		IInteract::Execute_OnInteract(Actor, Player);
		return;
	}

	// IInteractionBase (문 등)
	if (Actor->GetClass()->ImplementsInterface(UInteractionBase::StaticClass()))
	{
		IInteractionBase* Interactable = Cast<IInteractionBase>(Actor);
		if (Interactable)
		{
			Interactable->OnIneracted(Player);
		}
	}
}

// ══════════════════════════════════════════
//  외부 호출 (E키)
// ══════════════════════════════════════════

void UInteractionComponent::TryInteract()
{
	if (FocusedActor)
	{
		ExecuteInteraction(FocusedActor);
	}
}
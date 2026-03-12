#include "Gimmic_CY/DoorActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NavModifierComponent.h"
#include "NavAreas/NavArea_Default.h"
#include "NavAreas/NavArea_Null.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

// Sets default values
ADoorActor::ADoorActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Scene Root
	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	RootComponent = InteractionBox;

	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	Door->SetupAttachment(InteractionBox);
	Door->SetCollisionResponseToChannels(ECR_Block);

	DoorTimelineComp = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));

	NavModifier = CreateDefaultSubobject<UNavModifierComponent>(TEXT("NavModifier"));
	//NavModifier->SetupAttachment(Scene);
}

void ADoorActor::BeginPlay()
{
	Super::BeginPlay();

	InitialRotation = Door->GetRelativeRotation();
	NavModifier->SetAreaClass(UNavArea_Null::StaticClass());
	UpdateFunctionFloat.BindDynamic(this, &ADoorActor::UpdateTimelineComp);

	if (DoorTimelineFloatCurve)
	{
		DoorTimelineComp->AddInterpFloat(DoorTimelineFloatCurve, UpdateFunctionFloat);
	}

	//InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &ADoorActor::OnBeginOverlap);
	//InteractionBox->OnComponentEndOverlap.AddDynamic(this, &ADoorActor::OnEndOverlap);
}

void ADoorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADoorActor::UpdateTimelineComp(float Output)
{
	float NewYaw = FMath::Lerp(0.f, TargetYaw, Output);

	FRotator NewRotation = InitialRotation;
	NewRotation.Yaw += NewYaw;

	//FRotator NewRotation(0.f, Output, 0.f);
	Door->SetRelativeRotation(NewRotation);

}

void ADoorActor::OnBeginOverlap(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (Cast<APrototypeCharacter>(OtherActor))
	{
		IInteractionBase::Execute_OnIneractionRangeEntered(this);
	}
}

void ADoorActor::OnEndOverlap(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32)
{
	IInteractionBase::Execute_OnIneractionRangeExited(this);
}

void ADoorActor::OnIneractionRangeEntered_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, "I am an ineractable");
	// ���� ǥ�� ��
}

void ADoorActor::OnIneractionRangeExited_Implementation()
{
	// ���� ���� ��
}

void ADoorActor::OnIneracted_Implementation(APrototypeCharacter* Character)
{
	if (IInteractionBase::Execute_CanBeInteracted(this))
	{
		IInteractionBase::Execute_HandleInteraction(this, Character);
	}
}

void ADoorActor::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (!DoorTimelineFloatCurve || !Character || !bCanInteract)
		return;

	bCanInteract = false;
	Door->SetCollisionResponseToChannels(ECR_Ignore);

	FTimerHandle InteractionTimer;
	TWeakObjectPtr<ADoorActor> WeakThis(this);
	GetWorld()->GetTimerManager().SetTimer(InteractionTimer, FTimerDelegate::CreateLambda([WeakThis]() {
		WeakThis->bCanInteract = true;
		WeakThis->Door->SetCollisionResponseToChannels(ECR_Block);
		}), 1.0f, false);

	// ===== �÷��̾� ���� ��� =====
	FVector DoorLocation = GetActorLocation();
	FVector PlayerLocation = Character->GetActorLocation();

	FVector DoorForward = GetActorForwardVector();
	FVector ToPlayer = (PlayerLocation - DoorLocation).GetSafeNormal();

	float Dot = FVector::DotProduct(DoorForward, ToPlayer);

	// ===== ������ ���� ���� =====
	//TargetYaw = (Dot >= 0.f) ? 90.f : -90.f;
	//DoorTimelineComp->SetPlayRate(2.0f);

	if (!bIsOpen)
	{
		TargetYaw = (Dot >= 0.f) ? 90.f : -90.f;
		DoorTimelineComp->Play();

		NavModifier->SetAreaClass(UNavArea_Default::StaticClass()); // 통과 가능
	}
	else
	{
		DoorTimelineComp->Reverse();

		NavModifier->SetAreaClass(UNavArea_Null::StaticClass()); // 다시 막힘
	}

	bIsOpen = !bIsOpen;

}

EInteractionType ADoorActor::GetInteractionType_Implementation() const
{
	return EInteractionType::Door;
}

bool ADoorActor::SetBCanInteract(bool IsCanInteract)
{
	bCanInteract = IsCanInteract;
	return bCanInteract;
}

bool ADoorActor::GetBCanInteract() const
{
	return bCanInteract;
}
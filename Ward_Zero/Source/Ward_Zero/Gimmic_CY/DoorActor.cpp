#include "Gimmic_CY/DoorActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

// Sets default values
ADoorActor::ADoorActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root
	DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
	RootComponent = DoorFrame;

	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	Door->SetupAttachment(DoorFrame);

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetupAttachment(DoorFrame);
	InteractionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	DoorTimelineComp = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));
}

void ADoorActor::BeginPlay()
{
	Super::BeginPlay();

	InitialRotation = Door->GetRelativeRotation();

	UpdateFunctionFloat.BindDynamic(this, &ADoorActor::UpdateTimelineComp);

	if (DoorTimelineFloatCurve)
	{
		DoorTimelineComp->AddInterpFloat(DoorTimelineFloatCurve, UpdateFunctionFloat);
	}

	InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &ADoorActor::OnBeginOverlap);
	InteractionBox->OnComponentEndOverlap.AddDynamic(this, &ADoorActor::OnEndOverlap);
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
	// 위젯 표시 등
}

void ADoorActor::OnIneractionRangeExited_Implementation()
{
	// 위젯 숨김 등
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
	if (!DoorTimelineFloatCurve || !Character)
		return;

	// ===== 플레이어 방향 계산 =====
	FVector DoorLocation = GetActorLocation();
	FVector PlayerLocation = Character->GetActorLocation();

	FVector DoorForward = GetActorForwardVector();
	FVector ToPlayer = (PlayerLocation - DoorLocation).GetSafeNormal();

	float Dot = FVector::DotProduct(DoorForward, ToPlayer);

	// ===== 열리는 방향 결정 =====
	//TargetYaw = (Dot >= 0.f) ? 90.f : -90.f;
	//DoorTimelineComp->SetPlayRate(2.0f);

	if (!bIsOpen)
	{
		TargetYaw = (Dot >= 0.f) ? 90.f : -90.f;
		DoorTimelineComp->Play();
	}
	else
	{
		DoorTimelineComp->Reverse();
	}

	bIsOpen = !bIsOpen;

}

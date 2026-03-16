#include "Gimmic_CY/HealItemActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Components/Status/PlayerStatusComponent.h"

AHealItemActor::AHealItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 충돌 박스
	//CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetBoxExtent(FVector(20.0f, 20.0f, 20.0f));
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	// 약병 본체 메쉬
	BottleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BottleMesh")); // 이름 변경 권장
	BottleMesh->SetupAttachment(RootComponent);
	BottleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 약병 뚜껑 메쉬 
	CapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CapMesh"));
	CapMesh->SetupAttachment(BottleMesh); 
	CapMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 기둥
	MarkerPillar = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerPillar"));
	MarkerPillar->SetupAttachment(RootComponent);
	MarkerPillar->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	MarkerPillar->SetRelativeScale3D(FVector(0.05f, 0.05f, 0.5f));
	MarkerPillar->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 상호작용 위젯
	InteractWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractWidget"));
	InteractWidget->SetupAttachment(RootComponent);
	InteractWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
	InteractWidget->SetWidgetSpace(EWidgetSpace::Screen);
	InteractWidget->SetDrawSize(FVector2D(50.0f, 50.0f));

	PickUpPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PickUpPoint"));
	PickUpPoint->SetupAttachment(BottleMesh); 
	PickUpPoint->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
}

void AHealItemActor::BeginPlay()
{
	Super::BeginPlay();

	if (InteractWidget) InteractWidget->SetVisibility(false);

	if (CollisionBox)
	{
		CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AHealItemActor::OnOverlapBegin);
		CollisionBox->OnComponentEndOverlap.AddDynamic(this, &AHealItemActor::OnOverlapEnd);
	}
}

void AHealItemActor::OnIneracted_Implementation(APrototypeCharacter* Character)
{
	if (Character && Character->StatusComp)
	{
		// 회복약 개수 증가 시도
		if (Character->StatusComp->AddHealingItem(1))
		{
			UE_LOG(LogTemp, Warning, TEXT("Heal Item Acquired!"));
			Destroy();
		}
	}
}

EInteractionType AHealItemActor::GetInteractionType_Implementation() const
{
	// Enum에 Heal이 없다면 임시로 Ammo 사용, 있다면 Heal로 변경
	return EInteractionType::Heal;
}

// 위젯 가시성 제어
void AHealItemActor::OnIneractionRangeEntered_Implementation()
{
	if (InteractWidget) InteractWidget->SetVisibility(true);
}

void AHealItemActor::OnIneractionRangeExited_Implementation()
{
	if (InteractWidget) InteractWidget->SetVisibility(false);
}

void AHealItemActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(APrototypeCharacter::StaticClass()))
	{
		IInteractionBase::Execute_OnIneractionRangeEntered(this);
	}
}

void AHealItemActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor->IsA(APrototypeCharacter::StaticClass()))
	{
		IInteractionBase::Execute_OnIneractionRangeExited(this);
	}
}

// 인터페이스 필수 가상 함수들
bool AHealItemActor::SetBCanInteract(bool IsCanInteract) { bCanInteract = IsCanInteract; return bCanInteract; }
bool AHealItemActor::GetBCanInteract() const { return bCanInteract; }
void AHealItemActor::HiddenActor() {}

FVector AHealItemActor::GetInteractionTargetLocation_Implementation() const
{
	// 하드코딩 대신 컴포넌트의 실제 월드 위치를 반환!
	if (PickUpPoint)
	{
		return PickUpPoint->GetComponentLocation();
	}
	return GetActorLocation(); // 예외 처리
}
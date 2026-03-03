#include "Weapon/Magazine/MagazineBase.h"
#include "Components/BoxComponent.h"

AMagazineBase::AMagazineBase()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	RootComponent = BoxComp;

	//손에 붙어있을 때는 물리엔진 끄기 
	BoxComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BoxComp->SetSimulatePhysics(false);
	BoxComp->SetUseCCD(true);

	MagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MagMesh"));
	MagMesh->SetupAttachment(RootComponent);
	MagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMagazineBase::Drop()
{
	// 부모(캐릭터 Hand)로부터 떼어내기
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (BoxComp)
	{
		BoxComp->SetCollisionProfileName(TEXT("PhysicsActor"));
		BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		BoxComp->SetSimulatePhysics(true);
	}

	// 일정 시간 후 탄창 삭제 
	SetLifeSpan(10.0f);
}

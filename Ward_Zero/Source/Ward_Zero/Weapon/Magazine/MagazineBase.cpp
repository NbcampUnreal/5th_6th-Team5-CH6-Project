#include "Weapon/Magazine/MagazineBase.h"
#include "Components/BoxComponent.h"

AMagazineBase::AMagazineBase()
{
	PrimaryActorTick.bCanEverTick = false;

	MagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MagMesh"));
	SetRootComponent(RootComponent);
	MagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MagMesh->SetSimulatePhysics(false);

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	BoxComp->SetupAttachment(RootComponent);
	BoxComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMagazineBase::Drop()
{
	//다 쓴 탄창을 바닥에 버릴 떄 
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (MagMesh)
	{
		MagMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
		MagMesh->SetSimulatePhysics(true); //물리 엔진 활성화 
		MagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MagMesh->GetBodyInstance()->bUseCCD = true;//연속 충돌 감지 
	}
	SetLifeSpan(5.0f); //5초 뒤 소멸 
}

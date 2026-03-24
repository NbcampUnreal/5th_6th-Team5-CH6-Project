#include "Gimmic_CY/Object/Door/SingleDoor.h"
#include "NavModifierComponent.h"
#include "NavAreas/NavArea_Default.h"
#include "NavAreas/NavArea_Null.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

ASingleDoor::ASingleDoor()
{
	if (Mesh)
	{
		Mesh->SetGenerateOverlapEvents(true);
		Mesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	}
}

void ASingleDoor::BeginPlay()
{
	Super::BeginPlay();
	
	if (Mesh)
	{
		InitialRotation = Mesh->GetRelativeRotation();
		Mesh->OnComponentBeginOverlap.AddDynamic(this, &ASingleDoor::OnOverLapBegin);
		Mesh->OnComponentEndOverlap.AddDynamic(this, &ASingleDoor::OnOverLapEnd);
		UMaterialInterface* BaseM = Mesh->GetMaterial(0);
		if (BaseM)
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(BaseM,this);
			Mesh->SetMaterial(0, DynamicMaterial);
		}
	}
	
	
	
	if (DoorAnimationType == ESingleDoorAnimationType::SingleDoor_Push)
	{
		TargetYaw = -90.f;
	}else
	{
		TargetYaw = 90.f;
	}
}

void ASingleDoor::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (!DoorTimelineFloatCurve || !Character || !bCanInteract)
		return;
	
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	FTimerHandle InteractionTimer;
	TWeakObjectPtr<ASingleDoor> WeakThis(this);
	GetWorld()->GetTimerManager().SetTimer(InteractionTimer, FTimerDelegate::CreateLambda([WeakThis]() {
		WeakThis->Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		}), 1.0f, false);

	FVector DoorLocation = GetActorLocation();
	FVector PlayerLocation = Character->GetActorLocation();

	FVector DoorForward = GetActorForwardVector();
	FVector ToPlayer = (PlayerLocation - DoorLocation).GetSafeNormal();

	float Dot = FVector::DotProduct(DoorForward, ToPlayer);

	

	bIsOpen = true;
	
	Activate();
	

	

}

EInteractionType ASingleDoor::GetInteractionType_Implementation() const
{
	//return EInteractionType::SingleDoor;
	return EInteractionType::Door;
}

ESingleDoorAnimationType ASingleDoor::GetSingleDoorAnimationType() const
{
	return DoorAnimationType;
}

void ASingleDoor::OnOverLapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this))
	{
		APrototypeCharacter* Player = Cast<APrototypeCharacter>(OtherActor);
		if (Player && Mesh)
		{
			DynamicMaterial->SetScalarParameterValue(FName("OpacityParam"),0.3f);
		}
	}
}

void ASingleDoor::OnOverLapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && (OtherActor != this))
	{
		APrototypeCharacter* Player = Cast<APrototypeCharacter>(OtherActor);
		if (Player && Mesh)
		{
			DynamicMaterial->SetScalarParameterValue(FName("OpacityParam"),1.0f);
		}
	}
}

void ASingleDoor::UpdateTimelineComp(float Output)
{
	float NewYaw = FMath::Lerp(0.f, TargetYaw, Output);

	FRotator NewRotation = InitialRotation;
	NewRotation.Yaw += NewYaw;
	Mesh->SetRelativeRotation(NewRotation);
}

ESingleDoorAnimationType ASingleDoor::GetDoorAnimationType()
{
	return DoorAnimationType;
}

void ASingleDoor::OpenDoor()
{
	Super::OpenDoor();
	DoorTimelineComp->Play();
	
}

void ASingleDoor::CloseDoor()
{
	
	Super::CloseDoor();
	
	DoorTimelineComp->Reverse();
	bIsOpen = false;
	
}

#pragma once

#include "CoreMinimal.h"
#include "Gimmic_CY/Base/DoorBase.h"
#include "SafeActor.generated.h"

UCLASS()
class WARD_ZERO_API ASafeActor : public ADoorBase
{
	GENERATED_BODY()
	
public:
	ASafeActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Door;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Pivot;

	FRotator InitialRotation;

	float TargetYaw = -100.f;

	virtual void UpdateTimelineComp(float Output) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> Items;

public:
	// ===== IGimmickInterface =====
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;
	
	virtual void Activate() override;
};

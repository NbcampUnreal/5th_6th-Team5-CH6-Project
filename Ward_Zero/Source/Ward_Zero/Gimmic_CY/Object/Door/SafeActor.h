#pragma once

#include "CoreMinimal.h"
#include "Gimmic_CY/Object/Door/DoorBase.h"
#include "SafeActor.generated.h"

UCLASS()
class WARD_ZERO_API ASafeActor : public ADoorBase
{
	GENERATED_BODY()
	
public:
	ASafeActor();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Door;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Pivot;

	FRotator InitialRotation;

	float TargetYaw = -100.f;

	virtual void UpdateTimelineComp(float Output) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "In Items"), Category = "Item")
	TArray<AActor*> Items;

	
	bool bVanishMagic = false;
	
	UFUNCTION(CallInEditor, Category = "Editor")
	void DoorVanishMagic();
	
public:
	// ===== IGimmickInterface =====
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;
	
	virtual void Activate() override;
};

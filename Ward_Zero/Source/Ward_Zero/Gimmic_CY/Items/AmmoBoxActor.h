#pragma once

#include "CoreMinimal.h"
#include "Gimmic_CY/Base/ItemBase.h"
#include "AmmoBoxActor.generated.h"

class UWidgetComponent;
class UStaticMeshComponent;

UCLASS()
class WARD_ZERO_API AAmmoBoxActor : public AItemBase
{
	GENERATED_BODY()
	
public:
	AAmmoBoxActor();

protected:
	virtual void BeginPlay() override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo Loot")
	int32 AmmoAmount = 15;

	// 어떤 무기의 총알인지? (1: 권총, 2: SMG)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo Loot")
	int32 TargetWeaponIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	EInteractionType InteractType = EInteractionType::Ammo;

	// [추가!] 멀리서 보이는 상시 빨간 기둥 (에디터에서 원기둥 지정)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MarkerPillar;

	// [추가!] 가까이 가면 뜨는 상호작용 동그라미 위젯
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* InteractWidget;

	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;
	virtual bool CanBeInteracted_Implementation() const override { return true; }
	virtual EInteractionType GetInteractionType_Implementation() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> PickUpPoint;

	FVector GetInteractionTargetLocation_Implementation() const;
};

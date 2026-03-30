#pragma once

#include "CoreMinimal.h"
#include "Gimmic_CY/Items/ItemBase.h"
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

	

	// � ������ �Ѿ�����? (1: ����, 2: SMG)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo Loot")
	int32 TargetWeaponIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	EInteractionType InteractType = EInteractionType::Ammo;
	

	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;
	virtual bool CanBeInteracted_Implementation() const override { return true; }
	virtual EInteractionType GetInteractionType_Implementation() const override;
	
};

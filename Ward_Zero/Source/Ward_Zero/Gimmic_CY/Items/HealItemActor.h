#pragma once

#include "CoreMinimal.h"
#include "Gimmic_CY/Base/ItemBase.h"
#include "HealItemActor.generated.h"

class UBoxComponent;
class UWidgetComponent;

UCLASS()
class WARD_ZERO_API AHealItemActor : public AItemBase
{
	GENERATED_BODY()

public:
	AHealItemActor();

protected:
	virtual void BeginPlay() override;

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* CapMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MarkerPillar;

	// 오버랩 이벤트 함수
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;
	virtual EInteractionType GetInteractionType_Implementation() const override;
	virtual void HiddenActor() override;
	
};


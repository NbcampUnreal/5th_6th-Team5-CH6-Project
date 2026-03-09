// DocumentPickupActor.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmic_CY/InteractionBase.h"
#include "DocumentPickupActor.generated.h"

class UDocumentData;
class UStaticMeshComponent;
class UBoxComponent;

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API ADocumentPickupActor : public AActor, public IInteractionBase
{
	GENERATED_BODY()

public:
	ADocumentPickupActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	UDocumentData* DocumentData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	bool bCanInteract = true;

	// ===== IInteractionBase 구현 =====
	virtual void OnIneracted_Implementation(APrototypeCharacter* Character) override;
	virtual bool CanBeInteracted_Implementation() const override;
	virtual EInteractionType GetInteractionType_Implementation() const override;
	virtual void OnIneractionRangeEntered_Implementation() override {}
	virtual void OnIneractionRangeExited_Implementation() override {}
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override {}

protected:
	/** 라인트레이스 감지용 콜리전 박스 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* InteractionBox;

	/** 서류 비주얼 메시 (콜리전 없음) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DocumentMesh;

	virtual void BeginPlay() override;
};

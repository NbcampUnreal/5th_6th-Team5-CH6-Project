#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmic_CY/Base/InteractionBase.h"
#include "ItemBase.generated.h"

class UWidgetComponent;
class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class WARD_ZERO_API AItemBase : public AActor, public IInteractionBase
{
	GENERATED_BODY()
	
public:	
	AItemBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;


	// ===== IGimmickInterface =====
public:
	virtual void OnIneractionRangeEntered_Implementation() override;
	virtual void OnIneractionRangeExited_Implementation() override;
	virtual void OnIneracted_Implementation(APrototypeCharacter* Character) override;
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;
	virtual bool CanBeInteracted_Implementation() const override { return true; }
	virtual EInteractionType GetInteractionType_Implementation() const override;
	virtual bool SetBCanInteract(bool IsCanInteract) override;
	virtual bool GetBCanInteract() const override;
	virtual void PostActorCreated() override;
	
	virtual void ShowPressEWidget_Implementation() override;
	virtual void HidePressEWidget_Implementation() override;

	// ===== SaveInterface =====
	//virtual void SaveActorState(class UWardSaveGame* SaveData) override;
	//virtual void LoadActorState(class UWardSaveGame* SaveData) override;

	FVector GetInteractionTargetLocation_Implementation() const override;

	virtual void HiddenActor();
	
	UPROPERTY(EditInstanceOnly)
	bool bDefaultInteractable = true;
	
	bool bActivated = false;
	
	FRotator GetInHandTransform() const { return InHandRotator; }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	class UWidgetComponent* InteractWidget;
	
protected:
	UPROPERTY(EditInstanceOnly)
	FGuid ActorID;

	UPROPERTY()
	bool bCollected = false;

	// Mesh
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Setting")
	FRotator InHandRotator;
	
	
	
	UPROPERTY(VisibleAnywhere)
	USceneComponent* PickUpPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* CollisionBox;
};

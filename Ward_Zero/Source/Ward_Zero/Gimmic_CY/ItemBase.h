#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmic_CY/InteractionBase.h"
#include "SaveInterface.h"
#include "ItemBase.generated.h"

class UStaticMeshComponent;

UCLASS()
class WARD_ZERO_API AItemBase : public AActor, public IInteractionBase, public ISaveInterface
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
	virtual void HiddenActor() override;
	virtual void PostActorCreated() override;

	// ===== SaveInterface =====
	virtual FGuid GetActorID() const override;
	virtual void SaveActorState(class UWardSaveGame* SaveData) override;
	virtual void LoadActorState(class UWardSaveGame* SaveData) override;


protected:
	UPROPERTY(EditInstanceOnly)
	FGuid ActorID;

	UPROPERTY()
	bool bCollected = false;

	// 카드키 메시
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;
};

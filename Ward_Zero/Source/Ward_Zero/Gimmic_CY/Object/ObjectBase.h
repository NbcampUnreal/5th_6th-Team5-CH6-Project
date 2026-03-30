#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmic_CY/Interface/InteractionBase.h"
#include "ObjectBase.generated.h"

class ABaseZombieActivateTrigger;
class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class WARD_ZERO_API AObjectBase : public AActor, public IInteractionBase
{
	GENERATED_BODY()
	
public:	
	AObjectBase();

protected:
	virtual void BeginPlay() override;
	
	bool bGamePlay = false;

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
	UFUNCTION(BlueprintCallable)
	virtual bool SetBCanInteract(bool IsCanInteract) override;
	UFUNCTION(BlueprintCallable)
	virtual bool GetBCanInteract() const override;
	virtual void ShowPressEWidget_Implementation() override;
	virtual void HidePressEWidget_Implementation() override;
	virtual void SaveActorState() const override;
	
	virtual void PostActorCreated() override;
	
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UStaticMeshComponent* Lamp;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ChangeColorLampRed();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ChangeColorLampGreen();
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void Activate();
	
	bool bActivated = false;

	FVector GetInteractionTargetLocation_Implementation() const;
	// Mesh
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;
	
	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicMaterial;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	class UWidgetComponent* InteractWidget;
	
	
	UPROPERTY(EditInstanceOnly)
	FGuid ActorID;
	
	UPROPERTY(EditAnywhere,Category = "Setting")
	bool bDefaultInteractable = true;
	
	virtual FVector GetIKTargetLocation_Implementation() const override;
	
	UPROPERTY(VisibleAnywhere)
	USceneComponent* PickUpPoint;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	USceneComponent* PullPoint;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category= "Setting")
	TArray<AActor*> ActivatingTriggers;
	
	void ActivateTriggers();
#if WITH_EDITOR
	virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override;
	virtual void PostEditImport() override;
	
	UFUNCTION(CallInEditor, Category = "Editor")
	void RegenerateAllObjectIDsInLevel();

#endif
};

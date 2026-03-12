#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "Gimmic_CY/InteractionBase.h"
#include "Gimmick_Lever.generated.h"

class UStaticMeshComponent;
class UCurveFloat;
class APrototypeCharacter;

UCLASS()
class WARD_ZERO_API AGimmick_Lever : public AActor, public IInteractionBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGimmick_Lever();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Lever_Frame;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Lever_Handle;

	UPROPERTY(EditAnywhere)
	UCurveFloat* LeverCurve;

	FOnTimelineFloat UpdateFunctionFloat;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual bool CanBeInteracted_Implementation() const override;
	virtual void OnIneracted_Implementation(APrototypeCharacter* Character) override;
	virtual void OnIneractionRangeEntered_Implementation() override;
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;
	virtual EInteractionType GetInteractionType_Implementation() const override;
	virtual bool SetBCanInteract(bool IsCanInteract) override;
	virtual bool GetBCanInteract() const override;
};

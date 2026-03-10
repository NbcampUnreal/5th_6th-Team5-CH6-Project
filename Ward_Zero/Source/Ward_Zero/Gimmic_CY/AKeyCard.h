#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmic_CY/InteractionBase.h"
#include "AKeyCard.generated.h"

class UStaticMeshComponent;
class APrototypeCharacter;
class UBoxComponent;

UCLASS()
class WARD_ZERO_API AAKeyCard : public AActor, public IInteractionBase
{
	GENERATED_BODY()
	
public:	
	AAKeyCard();

protected:
	virtual void BeginPlay() override;

public:	
	// 카드키 메시
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* KeyMesh;

	// Interaction Range
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* InteractionBox;

	// 인터페이스 함수
	virtual void OnIneracted_Implementation(APrototypeCharacter* Character) override;
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;
	virtual bool CanBeInteracted_Implementation() const override { return true; }

	virtual EInteractionType GetInteractionType_Implementation() const override;

};

#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "KeyCardActor.generated.h"

class UStaticMeshComponent;
UCLASS()
class WARD_ZERO_API AKeyCardActor : public AItemBase
{
	GENERATED_BODY()
	
public:
	AKeyCardActor();

	// 인터페이스 함수
	//virtual void OnIneracted_Implementation(APrototypeCharacter* Character) override;
	//virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;
	virtual bool CanBeInteracted_Implementation() const override { return true; }
	virtual EInteractionType GetInteractionType_Implementation() const override;
	//virtual void HiddenActor() override;
};

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FootstepComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WARD_ZERO_API UFootstepComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFootstepComponent();

	void PlayFootstep(FName FootBoneName);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	class USoundBase* Sound_DefaultStep;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	class USoundBase* Sound_ConcreteStep;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	class USoundBase* Sound_Marble;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	class USoundBase* Sound_MetalStep;

	UPROPERTY(EditAnywhere, Category = "Data")
	TObjectPtr<class UFootstepData> FootstepDataAsset;
};
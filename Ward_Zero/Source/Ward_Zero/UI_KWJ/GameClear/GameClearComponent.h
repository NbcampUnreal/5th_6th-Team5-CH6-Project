#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameClearComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WARD_ZERO_API UGameClearComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGameClearComponent();

	void ActivateGameClear(class APrototypeCharacter* Player);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameClear")
	float TestPlayTimeSeconds = 600.0f;
};

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveInterface.generated.h"

UINTERFACE(MinimalAPI)
class USaveInterface : public UInterface
{
	GENERATED_BODY()
};


class WARD_ZERO_API ISaveInterface
{
	GENERATED_BODY()

public:

	virtual FGuid GetActorID() const = 0;

	virtual void SaveActorState(class UWardSaveGame* SaveData) = 0;

	virtual void LoadActorState(class UWardSaveGame* SaveData) = 0;
};

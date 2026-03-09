#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagazineBase.generated.h"

class UBoxComponent;

UCLASS()
class WARD_ZERO_API AMagazineBase : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MagMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> BoxComp;

public:
	AMagazineBase();

	void Drop();
};

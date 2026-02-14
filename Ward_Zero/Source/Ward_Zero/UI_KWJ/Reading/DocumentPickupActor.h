// DocumentPickupActor.h

#pragma once

#include "CoreMinimal.h"
#include "Objects/BaseObject.h"
#include "DocumentPickupActor.generated.h"

class UDocumentData;

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API ADocumentPickupActor : public ABaseObject
{
	GENERATED_BODY()

public:
	ADocumentPickupActor();

	/** 이 액터가 가진 서류 데이터 (에디터에서 할당) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	UDocumentData* DocumentData;

	/** 서류를 열 수 있는지 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	bool bCanInteract = true;

	/** IInteract 인터페이스 구현 */
	virtual void OnInteract_Implementation(class APrototypeCharacter* Player) override;

protected:

	/** 서류 비주얼 메시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DocumentMesh;

	virtual void BeginPlay() override;
};

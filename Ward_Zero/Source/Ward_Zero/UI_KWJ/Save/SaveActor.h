// SaveActor.h
// 레벨에 배치하는 세이브 포인트 액터
// E키 상호작용 → 세이브 UI 표시

#pragma once

#include "CoreMinimal.h"
#include "Objects/BaseObject.h"
#include "SaveActor.generated.h"

class UStaticMeshComponent;

/**
 * 세이브 포인트 액터
 * ABaseObject를 상속하여 기존 IInteract 라인트레이스에 자동 연결
 */
UCLASS()
class WARD_ZERO_API ASaveActor : public ABaseObject
{
	GENERATED_BODY()

public:

	ASaveActor();

	/** 상호작용 시 세이브 UI 표시 */
	virtual void OnInteract_Implementation(APrototypeCharacter* Player) override;

protected:

	/** 세이브 포인트 메쉬 (책상, 타자기, 저널 등) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	UStaticMeshComponent* SaveMesh;
};

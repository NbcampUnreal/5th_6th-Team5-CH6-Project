#pragma once

#include "CoreMinimal.h"
#include "Gimmic_CY/ItemBase.h"
#include "HealItemActor.generated.h"

class UBoxComponent;
class UWidgetComponent;

UCLASS()
class WARD_ZERO_API AHealItemActor : public AItemBase
{
	GENERATED_BODY()

public:
	AHealItemActor();

protected:
	virtual void BeginPlay() override;

public:
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	//UBoxComponent* CollisionBox;

	// 약병 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BottleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* CapMesh;

	// 멀리서 보이는 빨간 기둥
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MarkerPillar;

	// 가까이 가면 뜨는 E키 위젯
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* InteractWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* PickUpPoint;

	// 오버랩 이벤트 함수
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 인터페이스 함수들 오버라이드
	virtual void OnIneractionRangeEntered_Implementation() override;
	virtual void OnIneractionRangeExited_Implementation() override;
	virtual void OnIneracted_Implementation(APrototypeCharacter* Character) override;
	virtual bool CanBeInteracted_Implementation() const override { return true; }
	virtual EInteractionType GetInteractionType_Implementation() const override;

	// 필수 가상 함수 구현
	virtual bool SetBCanInteract(bool IsCanInteract) override;
	virtual bool GetBCanInteract() const override;
	virtual void HiddenActor() override;

	FVector GetInteractionTargetLocation_Implementation() const;
};


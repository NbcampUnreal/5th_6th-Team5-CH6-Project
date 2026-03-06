// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmic_CY/InteractionBase.h"
#include "AmmoBox_DH.generated.h"

class UWidgetComponent;

UCLASS()
class WARD_ZERO_API AAmmoBox_DH : public AActor, public IInteractionBase
{
	GENERATED_BODY()

public:
	AAmmoBox_DH();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo Loot")
	int32 AmmoAmount = 15;

	// 어떤 무기의 총알인지? (1: 권총, 2: SMG)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo Loot")
	int32 TargetWeaponIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	EInteractionType InteractType = EInteractionType::Ammo;

	// [추가!] 멀리서 보이는 상시 빨간 기둥 (에디터에서 원기둥 지정)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MarkerPillar;

	// [추가!] 가까이 가면 뜨는 상호작용 동그라미 위젯
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* InteractWidget;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

#pragma region IInteractionBase 인터페이스 구현
	virtual void OnIneractionRangeEntered_Implementation() override;
	virtual void OnIneractionRangeExited_Implementation() override;

	virtual void OnIneracted_Implementation(APrototypeCharacter* Character) override;
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override {}
	virtual bool CanBeInteracted_Implementation() const override { return true; }
#pragma endregion
};

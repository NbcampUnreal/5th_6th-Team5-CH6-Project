#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "Gimmic_CY/InteractionBase.h"
#include "SafeActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UCurveFloat;
class UUserWidget;

UCLASS()
class WARD_ZERO_API ASafeActor : public AActor, public IInteractionBase
{
	GENERATED_BODY()
	
public:	
	ASafeActor();

protected:
	virtual void BeginPlay() override;

    // ===== Mesh =====
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* SafeBody;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Door;

    UPROPERTY(VisibleAnywhere)
    USceneComponent* Pivot;

    // ===== Timeline =====
    UPROPERTY()
    UTimelineComponent* SafeTimeline;

    UPROPERTY(EditAnywhere)
    UCurveFloat* OpenCurve;

    FOnTimelineFloat SafeUpdateFunctionFloat;

    UFUNCTION()
    void UpdateSafeRotation(float Alpha);

    FRotator InitialRotation;
    float TargetYaw = -100.f;

    // ===== 상태 =====
    UPROPERTY(EditAnywhere)
    bool bIsLocked = true;

    bool bIsOpened = false;

    // ===== 비밀번호 =====
    UPROPERTY(EditAnywhere)
    FString CorrectPassword = "1234";

    // ===== UI =====
    UPROPERTY(EditAnywhere)
    TSubclassOf<UUserWidget> PasswordWidgetClass;

    // Interaction Range
    UPROPERTY(VisibleAnywhere)
    UBoxComponent* InteractionBox;

    UPROPERTY()
    UUserWidget* ActiveWidget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> Items;

    // ===== IGimmickInterface =====
public:
    virtual void OnIneractionRangeEntered_Implementation() override;
    virtual void OnIneractionRangeExited_Implementation() override;
    virtual void OnIneracted_Implementation(APrototypeCharacter* Character) override;
    virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;
    virtual bool CanBeInteracted_Implementation() const override { return !bIsOpened;}
    virtual bool SetBCanInteract(bool IsCanInteract) override;
    virtual bool GetBCanInteract() const override;

protected:
    bool bCanInteract;

private:
    UFUNCTION()
    void OnBeginOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnEndOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);


};

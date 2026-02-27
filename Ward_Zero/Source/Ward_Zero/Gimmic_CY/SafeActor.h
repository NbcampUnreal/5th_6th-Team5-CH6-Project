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

    // ===== IGimmickInterface =====
public:
    virtual void OnIneractionRangeEntered() override;
    virtual void OnIneractionRangeExited() override;
    virtual void OnIneracted(APrototypeCharacter* Character) override;
    virtual void HandleInteraction(APrototypeCharacter* Character) override;
    virtual bool CanBeInteracted() const override { return true; }

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

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractTrigger.generated.h"

class UBoxComponent;
class UPlayerHUD;
class APlayerController;

UCLASS()
class WARD_ZERO_API AInteractTrigger : public AActor
{
	GENERATED_BODY()
	
public:	
	AInteractTrigger();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* TriggerBox;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	UPROPERTY()
	UUserWidget* InteractWidget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> InteractWidgetClass;

	UPROPERTY()
	TObjectPtr<UPlayerHUD> PlayerHUD;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPlayerHUD> PlayerHUDClass;

	UPROPERTY()
	TObjectPtr<AActor> Door;

	UPROPERTY()
	TObjectPtr<APlayerController> PCTestob;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DoorPasscode;

public:
	UFUNCTION()void SetDoorPasscode(int32 Passcode);
};

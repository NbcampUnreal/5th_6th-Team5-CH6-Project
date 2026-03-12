#include "Character/Components/QuickTurn/QuickTurnComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/Components/Combat/PlayerCombatComponent.h"

UQuickTurnComponent::UQuickTurnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UQuickTurnComponent::StartQuickTurn180()
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	UPlayerCombatComponent* Combat = Owner ? Owner->FindComponentByClass<UPlayerCombatComponent>() : nullptr;

	if (!Owner || bIsQuickTurning || (Combat && Combat->IsAiming())) return;

	bIsQuickTurning = true;
	TurnAlpha = 0.0f;
	TurnStartYaw = Owner->GetActorRotation().Yaw;
	ControlStartYaw = Owner->GetController() ? Owner->GetController()->GetControlRotation().Yaw : TurnStartYaw;

	// 무기 소지 상태에 따른 인덱스 (Pistol: 6, Unarmed: 2)
	TurnIndex = (Combat && Combat->IsPistolEquipped()) ? 6 : 2;

	Owner->GetCharacterMovement()->StopMovementImmediately();
	SetComponentTickEnabled(true);
}

void UQuickTurnComponent::StopQuickTurn()
{
	bIsQuickTurning = false;
	TurnIndex = 0;
	SetComponentTickEnabled(false);
}

void UQuickTurnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsQuickTurning) return;

	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return;

	TurnAlpha += DeltaTime / Duration180;

	float SmoothAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, FMath::Clamp(TurnAlpha, 0.0f, 1.0f), 2.0f);

	// Actor 회전
	FRotator NewRot = Owner->GetActorRotation();
	NewRot.Yaw = FMath::Lerp(TurnStartYaw, TurnStartYaw + 180.0f, SmoothAlpha);
	Owner->SetActorRotation(NewRot);

	// Controller 회전
	if (Owner->GetController())
	{
		FRotator NewControlRot = Owner->GetController()->GetControlRotation();
		NewControlRot.Yaw = FMath::Lerp(ControlStartYaw, ControlStartYaw + 180.0f, SmoothAlpha);
		Owner->GetController()->SetControlRotation(NewControlRot);
	}

	if (TurnAlpha >= 1.0f) StopQuickTurn();
}


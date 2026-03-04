#include "UI_KWJ/GameClear/GameClearComponent.h"
#include "UI_KWJ/GameClear/GameClearSubsystem.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Ward_Zero.h"

UGameClearComponent::UGameClearComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGameClearComponent::ActivateGameClear(APrototypeCharacter* Player)
{
	if (!Player) return;

	UE_LOG(LogWard_Zero, Log, TEXT("GameClear 활성화: %s"), *GetOwner()->GetName());

	APlayerController* PC = Cast<APlayerController>(Player->GetController());
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	UGameClearSubsystem* ClearSys = LP->GetSubsystem<UGameClearSubsystem>();
	if (ClearSys)
	{
		ClearSys->ShowGameClear(TestPlayTimeSeconds);
	}
}

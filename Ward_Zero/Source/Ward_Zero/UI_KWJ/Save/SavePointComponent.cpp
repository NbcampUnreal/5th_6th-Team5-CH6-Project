// SavePointComponent.cpp

#include "UI_KWJ/Save/SavePointComponent.h"
#include "UI_KWJ/Save/SaveSubsystem.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Ward_Zero.h"

USavePointComponent::USavePointComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USavePointComponent::ActivateSavePoint(APrototypeCharacter* Player)
{
	if (!Player) return;

	UE_LOG(LogWard_Zero, Log, TEXT("세이브 포인트 활성화: %s"), *GetOwner()->GetName());

	APlayerController* PC = Cast<APlayerController>(Player->GetController());
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	USaveSubsystem* SaveSub = LP->GetSubsystem<USaveSubsystem>();
	if (SaveSub)
	{
		SaveSub->ShowSaveUI();
	}
}

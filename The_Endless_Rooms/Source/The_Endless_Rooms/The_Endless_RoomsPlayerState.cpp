#include "The_Endless_RoomsPlayerState.h"
#include "Net/UnrealNetwork.h"

void AThe_Endless_RoomsPlayerState::Server_ToggleReady_Implementation()
{
    SetReady_Server(!bReady);

    UE_LOG(LogTemp, Warning, TEXT("Ready Toggled : %s -> %d"), *GetPlayerName(), bReady);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green,
            FString::Printf(TEXT("[SERVER] %s Ready=%d"), *GetPlayerName(), bReady));
    }
}

void AThe_Endless_RoomsPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AThe_Endless_RoomsPlayerState, bReady);
}

void AThe_Endless_RoomsPlayerState::SetReady_Server(bool bNewReady)
{
    if (!HasAuthority())
    {
        return;
    }

    bReady = bNewReady;
    OnReadyChanged.Broadcast(bReady);
}

void AThe_Endless_RoomsPlayerState::Server_SetReady_Implementation(bool bNewReady)
{
    SetReady_Server(bNewReady);
}

void AThe_Endless_RoomsPlayerState::OnRep_Ready()
{
    OnReadyChanged.Broadcast(bReady);
}
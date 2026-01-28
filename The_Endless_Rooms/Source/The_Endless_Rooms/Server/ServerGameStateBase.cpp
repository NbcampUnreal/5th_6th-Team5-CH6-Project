#include "Server/ServerGameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "The_Endless_RoomsPlayerState.h"

void AServerGameStateBase::Multicast_ReceiveChat_Implementation(const FString& Sender, const FString& Message)
{
    OnChatMessageReceived.Broadcast(Sender, Message);

    UE_LOG(LogTemp, Warning, TEXT("Chat_Multicast = %s : %s"), *Sender, *Message);
}

void AServerGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AServerGameStateBase, HostPlayerState);
    DOREPLIFETIME(AServerGameStateBase, SelectedMapPath);
    DOREPLIFETIME(AServerGameStateBase, Phase);
}

void AServerGameStateBase::SetHost_Server(AThe_Endless_RoomsPlayerState* NewHost)
{
    if (!HasAuthority())
    {
        return;
    }

    HostPlayerState = NewHost;
    OnHostChanged.Broadcast(HostPlayerState);
    ForceNetUpdate();
}

void AServerGameStateBase::SetSelectedMap_Server(const FString& NewPath)
{
    SelectedMapPath = NewPath;
    OnSelectedMapChanged.Broadcast(SelectedMapPath);
    ForceNetUpdate();

}

void AServerGameStateBase::SetPhase_Server(ELobbyPhase NewPhase)
{
    Phase = NewPhase;
    OnPhaseChanged.Broadcast(Phase);
    ForceNetUpdate();

}

void AServerGameStateBase::OnRep_HostPlayerState()
{
    OnHostChanged.Broadcast(HostPlayerState);
}

void AServerGameStateBase::OnRep_SelectedMapPath()
{
    OnSelectedMapChanged.Broadcast(SelectedMapPath);
}

void AServerGameStateBase::OnRep_Phase()
{
    OnPhaseChanged.Broadcast(Phase);
}
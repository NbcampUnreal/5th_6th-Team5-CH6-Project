#include "The_Endless_Rooms_Controller.h"

#include "The_Endless_RoomsPlayerState.h"
#include "Server/ServerGameStateBase.h"
#include "Server/LobbyGameModeBase.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

void AThe_Endless_Rooms_Controller::RequestStartGame()
{
    if (IsLocalController())
    {
        ServerRequestStartGame();
    }
}

void AThe_Endless_Rooms_Controller::RequestRestart()
{
    if (IsLocalController())
    {
        ServerRequestRestart();
    }
}

void AThe_Endless_Rooms_Controller::RequestSelectMap(const FString& MapPath)
{
    if (IsLocalController())
    {
        ServerRequestSelectMap(MapPath);
    }
}

void AThe_Endless_Rooms_Controller::RequestSendChat(const FString& Message)
{
    if (IsLocalController())
    {
        ServerRequestSendChat(Message);
    }
}

void AThe_Endless_Rooms_Controller::ServerRequestSendChat_Implementation(const FString& Message)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    AServerGameStateBase* GS = World->GetGameState<AServerGameStateBase>();
    if (!GS)
    {
        return;
    }

    FString Clean = Message;
    Clean.TrimStartAndEndInline();

    if (Clean.IsEmpty())
    {
        return;
    }
    if (Clean.Len() > 200)
    {
        Clean.LeftInline(200);
    }

    FString Sender = TEXT("Player");
    if (PlayerState)
    {
        Sender = PlayerState->GetPlayerName();
    }

    GS->Multicast_ReceiveChat(Sender, Clean);

    UE_LOG(LogTemp, Warning, TEXT("Chat_Server = %s : %s"), *Sender, *Clean);
}

void AThe_Endless_Rooms_Controller::ServerRequestStartGame_Implementation()
{
    if (AGameModeBase* GM = GetWorld()->GetAuthGameMode())
    {
        if (ALobbyGameModeBase* LGM = Cast<ALobbyGameModeBase>(GM))
        {
            LGM->HandleStartRequest(this);
        }
    }
}

void AThe_Endless_Rooms_Controller::ServerRequestRestart_Implementation()
{
    if (AGameModeBase* GM = GetWorld()->GetAuthGameMode())
    {
        if (ALobbyGameModeBase* LGM = Cast<ALobbyGameModeBase>(GM))
        {
            LGM->HandleRestartRequest(this);
        }
    }
}

void AThe_Endless_Rooms_Controller::ServerRequestSelectMap_Implementation(const FString& MapPath)
{
    if (AGameModeBase* GM = GetWorld()->GetAuthGameMode())
    {
        if (ALobbyGameModeBase* LGM = Cast<ALobbyGameModeBase>(GM))
        {
            LGM->HandleSelectMapRequest(this, MapPath);
        }
    }
}

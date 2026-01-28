#include "Server/LobbyGameModeBase.h"
#include "ServerGameStateBase.h"
#include "The_Endless_RoomsPlayerState.h"
#include "The_Endless_Rooms_Controller.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

ALobbyGameModeBase::ALobbyGameModeBase()
{
    bUseSeamlessTravel = true;
    PlayerControllerClass = AThe_Endless_Rooms_Controller::StaticClass();
    PlayerStateClass = AThe_Endless_RoomsPlayerState::StaticClass();
    GameStateClass = AServerGameStateBase::StaticClass();
}

AServerGameStateBase* ALobbyGameModeBase::GetMPGS() const
{
    return GetGameState<AServerGameStateBase>();
}

bool ALobbyGameModeBase::IsHost(const APlayerController* PC) const
{
    const AServerGameStateBase* GS = GetMPGS();
    if (!GS || !PC || !PC->PlayerState)
    {
        return false;
    }
    return (PC->PlayerState == GS->HostPlayerState);
}

void ALobbyGameModeBase::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    AServerGameStateBase* GS = GetMPGS();
    if (!GS || !NewPlayer || !NewPlayer->PlayerState)
    {
        return;
    }
    //새로 들어온 플레이어의 준비 상태
    if (AThe_Endless_RoomsPlayerState* MPS = Cast<AThe_Endless_RoomsPlayerState>(NewPlayer->PlayerState))
    {
        MPS->SetReady_Server(false);
    }

    // 처음 로그인한 플레이를 호스트로 지정
    if (!GS->HostPlayerState)
    {
        if (AThe_Endless_RoomsPlayerState* PS = Cast<AThe_Endless_RoomsPlayerState>(NewPlayer->PlayerState))
        {
            GS->SetHost_Server(PS);
        }
    }

    // 로비 페이즈 보장
    if (GS->Phase != ELobbyPhase::Lobby)
    {
        GS->SetPhase_Server(ELobbyPhase::Lobby);
    }
}

void ALobbyGameModeBase::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    AServerGameStateBase* GS = GetMPGS();
    if (!GS)
    {
        return;
    }

    //// (선택) 호스트가 나가면 다음 플레이어를 호스트로 재지정
    //APlayerState* ExitingPS = Exiting ? Exiting->PlayerState : nullptr;
    //if (ExitingPS && ExitingPS == GS->HostPlayerState)
    //{
    //    AMPPlayerState* NewHost = nullptr;

    //    for (APlayerState* PS : GS->PlayerArray)
    //    {
    //        if (PS && PS != ExitingPS)
    //        {
    //            NewHost = Cast<AMPPlayerState>(PS);
    //            if (NewHost) break;
    //        }
    //    }

    //    GS->SetHost_Server(NewHost); // 없으면 nullptr
    //}
}

bool ALobbyGameModeBase::AreAllNonHostPlayersReady() const
{
    AServerGameStateBase* GS = GetMPGS();
    if (!GS)
    {
        return false;
    }

    int32 NonHostCount = 0;

    for (APlayerState* PS : GS->PlayerArray)
    {
        if (!PS)
        {
            continue;
        }
        if (PS == GS->HostPlayerState)
        {
            continue;
        }

        NonHostCount++;

        const AThe_Endless_RoomsPlayerState* MPS = Cast<AThe_Endless_RoomsPlayerState>(PS);
        if (!MPS || !MPS->bReady)
        {
            return false;
        }
    }

    // 참가자가 아예 없을 때 호스트 혼자 시작 허용 여부
    if (NonHostCount == 0)
    {
        return bAllowSoloStart;
    }

    return true;
}

void ALobbyGameModeBase::HandleSelectMapRequest(AThe_Endless_Rooms_Controller* Requester, const FString& MapPath)
{
    if (!Requester || !IsHost(Requester))
    {
        return;
    }

    AServerGameStateBase* GS = GetMPGS();
    if (!GS)
    {
        return;
    }

    if (!MapPath.IsEmpty())
    {
        GS->SetSelectedMap_Server(MapPath);
    }
}

void ALobbyGameModeBase::HandleStartRequest(AThe_Endless_Rooms_Controller* Requester)
{
    if (!Requester || !IsHost(Requester))
    {
        return;
    }

    AServerGameStateBase* GS = GetMPGS();
    if (!GS)
    {
        return;
    }

    // 로비에서만 시작
    if (GS->Phase != ELobbyPhase::Lobby)
    {
        return;
    }

    if (!AreAllNonHostPlayersReady())
    {
        return;
    }

    GS->SetPhase_Server(ELobbyPhase::InGame);

    const FString TargetMap = !GS->SelectedMapPath.IsEmpty() ? GS->SelectedMapPath : DefaultInGameMapPath;
    ServerTravelToInGame(TargetMap);
}

void ALobbyGameModeBase::HandleRestartRequest(AThe_Endless_Rooms_Controller* Requester)
{
    if (!Requester || !IsHost(Requester))
    {
        return;
    }
    // - 인게임이면 현재 맵 리로드 or 체크포인트 리셋
    // - 로비면 Ready 초기화 등
    AServerGameStateBase* GS = GetMPGS();
    if (!GS)
    {
        return;
    }

    //로비에서 Ready 모두 false로 되돌림
    if (GS->Phase == ELobbyPhase::Lobby)
    {
        for (APlayerState* PS : GS->PlayerArray)
        {
            if (AThe_Endless_RoomsPlayerState* MPS = Cast<AThe_Endless_RoomsPlayerState>(PS))
            {
                // 호스트 포함 리셋할지 정책 선택 가능
                MPS->SetReady_Server(false);
            }
        }
    }
    else
    {
        // 인게임 리스타트는 “현재 레벨 다시 열기” 같은 식으로 확장 가능
        // 여기서는 정책만 자리 잡아둠.
    }
}

void ALobbyGameModeBase::ServerTravelToInGame(const FString& MapPath)
{
    // Listen 서버 유지하려면 ?listen 붙여주는 게 안전
    FString TravelURL = MapPath;
    if (!TravelURL.Contains(TEXT("?listen")))
    {
        TravelURL += TEXT("?listen");
    }

    GetWorld()->ServerTravel(TravelURL, true);
}


// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameModeBase.generated.h"

class AThe_Endless_Rooms_Controller;
class AThe_Endless_RoomsPlayerState;
class AServerGameStateBase;

UCLASS()
class THE_ENDLESS_ROOMS_API ALobbyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ALobbyGameModeBase();

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    bool AreAllNonHostPlayersReady() const;

    // ===== 컨트롤러 RPC가 결국 들어오는 서버 처리 =====
    void HandleStartRequest(AThe_Endless_Rooms_Controller* Requester);
    void HandleRestartRequest(AThe_Endless_Rooms_Controller* Requester);
    void HandleSelectMapRequest(AThe_Endless_Rooms_Controller* Requester, const FString& MapPath);

protected:
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

private:
    AServerGameStateBase* GetMPGS() const;
    bool IsHost(const APlayerController* PC) const;

    // ===맵 경로===
    // 로비로 이동
    UPROPERTY(EditDefaultsOnly, Category = "Maps")
    FString LobbyMapPath = TEXT("/Game/Maps/Lobby");

    // 인게임 기본 맵
    UPROPERTY(EditDefaultsOnly, Category = "Maps")
    FString DefaultInGameMapPath = TEXT("/Game/Maps/CH1");

    // 호스트 혼자서도 시작 허용
    UPROPERTY(EditDefaultsOnly, Category = "Lobby")
    bool bAllowSoloStart = true;

    void ServerTravelToInGame(const FString& MapPath);
};


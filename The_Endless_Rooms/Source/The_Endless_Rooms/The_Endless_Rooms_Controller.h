#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "The_Endless_Rooms_Controller.generated.h"

UCLASS()
class THE_ENDLESS_ROOMS_API AThe_Endless_Rooms_Controller : public APlayerController
{
	GENERATED_BODY()

public:
    // ===== UI가 호출하는 함수들(블루프린트 버튼에서 호출) =====
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void RequestStartGame();

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void RequestRestart();

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void RequestSelectMap(const FString& MapPath);

    //채팅
    UFUNCTION(BlueprintCallable, Category = "Chat")
    void RequestSendChat(const FString& Message);

protected:
    // ===== 서버 RPC (요청 통로) =====
    UFUNCTION(Server, Reliable)
    void ServerRequestStartGame();

    UFUNCTION(Server, Reliable)
    void ServerRequestRestart();

    UFUNCTION(Server, Reliable)
    void ServerRequestSelectMap(const FString& MapPath);

    //채팅
    UFUNCTION(Server, Reliable)
    void ServerRequestSendChat(const FString& Message);
};

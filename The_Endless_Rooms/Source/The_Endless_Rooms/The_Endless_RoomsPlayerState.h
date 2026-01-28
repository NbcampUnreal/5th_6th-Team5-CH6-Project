#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "The_Endless_RoomsPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadyChanged, bool, bNewReady);

UCLASS()
class THE_ENDLESS_ROOMS_API AThe_Endless_RoomsPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
    UPROPERTY(ReplicatedUsing = OnRep_Ready, BlueprintReadOnly, Category = "Lobby")
    bool bReady = false;

    UPROPERTY(BlueprintAssignable, Category = "Lobby")
    FOnReadyChanged OnReadyChanged;

    //서버 전용 setter
    void SetReady_Server(bool bNewReady);
    //클라 -> 서버로 신호보내는 RPC
    UFUNCTION(Server, Reliable)
    void Server_SetReady(bool bNewReady);
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Lobby")
    void Server_ToggleReady();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    UFUNCTION()
    void OnRep_Ready();
};
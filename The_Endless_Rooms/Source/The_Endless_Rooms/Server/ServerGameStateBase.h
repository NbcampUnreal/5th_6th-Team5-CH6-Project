// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ServerGameStateBase.generated.h"

class AThe_Endless_RoomsPlayerState;

UENUM(BlueprintType)
enum class ELobbyPhase : uint8
{
	Lobby UMETA(DisplayName = "Lobby"),
	InGame UMETA(DisplayName = "InGame")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHostChanged, AThe_Endless_RoomsPlayerState*, NewHostPS);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectedMapChanged, const FString&, NewMapPath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, ELobbyPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChatMessageReceived, const FString&, Sender, const FString&, Message);

UCLASS()
class THE_ENDLESS_ROOMS_API AServerGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
    UPROPERTY(BlueprintAssignable, Category = "Chat")
    FOnChatMessageReceived OnChatMessageReceived;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_ReceiveChat(const FString& Sender, const FString& Message);

    // 누가 호스트인지(서버가 확정 → 클라에 복제)
    UPROPERTY(ReplicatedUsing = OnRep_HostPlayerState, BlueprintReadOnly, Category = "Lobby")
    AThe_Endless_RoomsPlayerState* HostPlayerState = nullptr;

    // 호스트가 선택한 다음 맵(서버가 확정 → 클라에 복제)
    UPROPERTY(ReplicatedUsing = OnRep_SelectedMapPath, BlueprintReadOnly, Category = "Lobby")
    FString SelectedMapPath;

    UPROPERTY(ReplicatedUsing = OnRep_Phase, BlueprintReadOnly, Category = "Lobby")
    ELobbyPhase Phase = ELobbyPhase::Lobby;

    UPROPERTY(BlueprintAssignable, Category = "Lobby")
    FOnHostChanged OnHostChanged;
    UPROPERTY(BlueprintAssignable, Category = "Lobby")
    FOnSelectedMapChanged OnSelectedMapChanged;
    UPROPERTY(BlueprintAssignable, Category = "Lobby")
    FOnPhaseChanged OnPhaseChanged;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 서버 전용 Setter들
    void SetHost_Server(AThe_Endless_RoomsPlayerState* NewHost);
    void SetSelectedMap_Server(const FString& NewPath);
    void SetPhase_Server(ELobbyPhase NewPhase);

protected:
    UFUNCTION() void OnRep_HostPlayerState();
    UFUNCTION() void OnRep_SelectedMapPath();
    UFUNCTION() void OnRep_Phase();
};
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "The_Endless_Rooms_MatchSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FTheEndlessRoomsSessionInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FString HostName;
    UPROPERTY(BlueprintReadOnly) int32 CurrentPlayers = 0;
    UPROPERTY(BlueprintReadOnly) int32 MaxPlayers = 0;
    UPROPERTY(BlueprintReadOnly) int32 PingMs = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionsFoundBP, const TArray<FTheEndlessRoomsSessionInfo>&, Sessions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionOpFailedBP, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSessionCreatedBP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSessionJoinedBP);

UCLASS(BlueprintType)
class THE_ENDLESS_ROOMS_API UThe_Endless_Rooms_MatchSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintAssignable) FOnSessionsFoundBP OnSessionsFound;
    UPROPERTY(BlueprintAssignable) FOnSessionOpFailedBP OnSessionOpFailed;
    UPROPERTY(BlueprintAssignable) FOnSessionCreatedBP OnSessionCreated;
    UPROPERTY(BlueprintAssignable) FOnSessionJoinedBP OnSessionJoined;

    UFUNCTION(BlueprintCallable) void HostSession(int32 MaxPlayers = 4, bool bIsLAN = false);
    UFUNCTION(BlueprintCallable) void FindSessions(int32 MaxResults = 50, bool bIsLAN = false);
    UFUNCTION(BlueprintCallable) void JoinSessionByIndex(int32 Index);
    UFUNCTION(BlueprintCallable) void DestroySession();

private:
    IOnlineSessionPtr SessionInterface;
    TSharedPtr<FOnlineSessionSearch> SessionSearch;
    FName SessionName = NAME_GameSession;

    // 맵 경로
    FString LobbyMapPath = TEXT("/Game/Maps/Lobby");

    // Destroy 후 재호스트용
    bool bRecreateAfterDestroy = false;
    int32 PendingMaxPlayers = 4;
    bool bPendingIsLAN = false;

    // ✅ UI 인덱스(필터된 리스트) -> 원본 SearchResults 인덱스 매핑
    TArray<int32> VisibleResultIndices;

private:
    void EnsureSessionInterface();
    void ClearDelegates();

    // Helpers
    FString GetOSSNameSafe() const;
    bool GetLocalPlayerNetId(FUniqueNetIdRepl& OutNetId, FString& OutFailReason) const;
    static FString JoinResultToString(EOnJoinSessionCompleteResult::Type Result);

    // Callbacks
    void OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful);
    void OnStartSessionComplete(FName InSessionName, bool bWasSuccessful);
    void OnFindSessionsComplete(bool bWasSuccessful);
    void OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result);
    void OnDestroySessionComplete(FName InSessionName, bool bWasSuccessful);

    FDelegateHandle CreateHandle;
    FDelegateHandle StartHandle;
    FDelegateHandle FindHandle;
    FDelegateHandle JoinHandle;
    FDelegateHandle DestroyHandle;
};

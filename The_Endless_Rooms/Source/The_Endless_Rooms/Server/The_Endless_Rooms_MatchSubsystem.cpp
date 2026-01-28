#include "Server/The_Endless_Rooms_MatchSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Online/OnlineSessionNames.h"

static const FName PROJECT_KEY(TEXT("PROJECT_TAG"));     // 우리 게임 식별용
static const FName BUILD_ID_KEY(TEXT("BUILD_ID"));       // 빌드/버전 호환 차단용
static const FName PRESENCE_KEY = SEARCH_PRESENCE;       // Presence 검색용

static const int32 BACKROOM_BUILD_ID = 1;

void UThe_Endless_Rooms_MatchSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    EnsureSessionInterface();
    UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem::Initialize] OSS=%s SessionIF=%d"),
        *GetOSSNameSafe(), SessionInterface.IsValid());
}

void UThe_Endless_Rooms_MatchSubsystem::Deinitialize()
{
    ClearDelegates();
    SessionSearch.Reset();
    SessionInterface.Reset();

    UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem::Deinitialize] cleared"));
    Super::Deinitialize();
}

FString UThe_Endless_Rooms_MatchSubsystem::GetOSSNameSafe() const
{
    const UGameInstance* GI = GetGameInstance();
    UWorld* World = GI ? GI->GetWorld() : nullptr;

    IOnlineSubsystem* OSS = World ? Online::GetSubsystem(World) : nullptr;
    return OSS ? OSS->GetSubsystemName().ToString() : TEXT("None");
}

bool UThe_Endless_Rooms_MatchSubsystem::GetLocalPlayerNetId(FUniqueNetIdRepl& OutNetId, FString& OutFailReason) const
{
    const UGameInstance* GI = GetGameInstance();
    UWorld* World = GI ? GI->GetWorld() : nullptr;
    if (!World)
    {
        OutFailReason = TEXT("No World (GameInstance World is null)");
        return false;
    }

    const ULocalPlayer* LP = World->GetFirstLocalPlayerFromController();
    if (!LP)
    {
        OutFailReason = TEXT("No LocalPlayer");
        return false;
    }

    if (!LP->GetPreferredUniqueNetId().IsValid())
    {
        OutFailReason = TEXT("LocalPlayer has no valid PreferredUniqueNetId");
        return false;
    }

    OutNetId = LP->GetPreferredUniqueNetId();
    return true;
}

FString UThe_Endless_Rooms_MatchSubsystem::JoinResultToString(EOnJoinSessionCompleteResult::Type Result)
{
    switch (Result)
    {
    case EOnJoinSessionCompleteResult::Success:               return TEXT("Success");
    case EOnJoinSessionCompleteResult::SessionIsFull:         return TEXT("SessionIsFull");
    case EOnJoinSessionCompleteResult::SessionDoesNotExist:   return TEXT("SessionDoesNotExist");
    case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:return TEXT("CouldNotRetrieveAddress");
    case EOnJoinSessionCompleteResult::AlreadyInSession:      return TEXT("AlreadyInSession");
    case EOnJoinSessionCompleteResult::UnknownError:          return TEXT("UnknownError");
    default:                                                  return TEXT("Unknown(enum)");
    }
}

void UThe_Endless_Rooms_MatchSubsystem::EnsureSessionInterface()
{
    if (SessionInterface.IsValid())
        return;

    UGameInstance* GI = GetGameInstance();
    UWorld* World = GI ? GI->GetWorld() : nullptr;
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[EnsureSessionInterface] World is null"));
        return;
    }

    IOnlineSubsystem* OSS = Online::GetSubsystem(World);
    if (!OSS)
    {
        UE_LOG(LogTemp, Error, TEXT("[EnsureSessionInterface] OnlineSubsystem is null"));
        return;
    }

    SessionInterface = OSS->GetSessionInterface();

    UE_LOG(LogTemp, Warning, TEXT("[EnsureSessionInterface] OSS=%s SessionIF=%d"),
        *OSS->GetSubsystemName().ToString(), SessionInterface.IsValid());
}

void UThe_Endless_Rooms_MatchSubsystem::ClearDelegates()
{
    if (!SessionInterface.IsValid())
        return;

    if (CreateHandle.IsValid())
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateHandle);
        CreateHandle.Reset();
    }
    if (StartHandle.IsValid())
    {
        SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartHandle);
        StartHandle.Reset();
    }
    if (FindHandle.IsValid())
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindHandle);
        FindHandle.Reset();
    }
    if (JoinHandle.IsValid())
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
        JoinHandle.Reset();
    }
    if (DestroyHandle.IsValid())
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroyHandle);
        DestroyHandle.Reset();
    }
}

void UThe_Endless_Rooms_MatchSubsystem::HostSession(int32 MaxPlayers, bool bIsLAN)
{
    EnsureSessionInterface();
    if (!SessionInterface.IsValid())
    {
        OnSessionOpFailed.Broadcast(TEXT("No SessionInterface (HostSession)"));
        return;
    }

    PendingMaxPlayers = MaxPlayers;
    bPendingIsLAN = bIsLAN;

    if (SessionInterface->GetNamedSession(SessionName))
    {
        bRecreateAfterDestroy = true;
        DestroySession();
        return;
    }

    FUniqueNetIdRepl NetId;
    FString FailReason;
    if (!GetLocalPlayerNetId(NetId, FailReason))
    {
        OnSessionOpFailed.Broadcast(FString::Printf(TEXT("HostSession: %s"), *FailReason));
        return;
    }

    FOnlineSessionSettings Settings;

    const bool bOnline = !bIsLAN;
    Settings.bIsLANMatch = bIsLAN;
    Settings.NumPublicConnections = MaxPlayers;
    Settings.bShouldAdvertise = true;
    Settings.bAllowJoinInProgress = true;

    // ✅ 공통 태그(필터 안정성)
    Settings.Set(PROJECT_KEY, FString(TEXT("THEENDLESSROOMS")), EOnlineDataAdvertisementType::ViaOnlineService);
    Settings.Set(BUILD_ID_KEY, BACKROOM_BUILD_ID, EOnlineDataAdvertisementType::ViaOnlineService);

    UE_LOG(LogTemp, Warning, TEXT("[HostSession] OSS=%s bIsLAN=%d MaxPlayers=%d NetId=%s Type=%s"),
        *GetOSSNameSafe(), (int32)bIsLAN, MaxPlayers,
        *NetId->ToString(), *NetId->GetType().ToString());

    if (bOnline)
    {
        Settings.bUsesPresence = true;
        Settings.bAllowJoinViaPresence = true;
        Settings.bUseLobbiesIfAvailable = true;
        Settings.bAllowInvites = true;

        // ✅ (추가) Steam 로비 검색에서 잘 먹는 키워드 필터
        Settings.Set(SEARCH_KEYWORDS, FString(TEXT("THEENDLESSROOMS")), EOnlineDataAdvertisementType::ViaOnlineService);

        // ✅ 맵/로비 정보 광고(선택)
        Settings.Set(SETTING_MAPNAME, LobbyMapPath, EOnlineDataAdvertisementType::ViaOnlineService);
    }
    else
    {
        Settings.bUsesPresence = false;
        Settings.bAllowJoinViaPresence = false;
        Settings.bUseLobbiesIfAvailable = false;
        Settings.bAllowInvites = false;

        // LAN이면 굳이 키워드 필요 없음
    }

    UE_LOG(LogTemp, Warning, TEXT("[HostSession] OSS=%s LAN=%d Online=%d Max=%d NetId=%s Type=%s Presence=%d Lobbies=%d"),
        *GetOSSNameSafe(), (int32)bIsLAN, (int32)bOnline, MaxPlayers,
        *NetId->ToString(), *NetId->GetType().ToString(),
        (int32)Settings.bUsesPresence, (int32)Settings.bUseLobbiesIfAvailable);

    CreateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &UThe_Endless_Rooms_MatchSubsystem::OnCreateSessionComplete));

    const bool bStarted = SessionInterface->CreateSession(*NetId, SessionName, Settings);
    if (!bStarted)
    {
        OnSessionOpFailed.Broadcast(TEXT("CreateSession() returned false"));
        ClearDelegates();
        return;
    }
}

void UThe_Endless_Rooms_MatchSubsystem::OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful)
{
    ClearDelegates();

    UE_LOG(LogTemp, Warning, TEXT("[CreateComplete] ok=%d name=%s OSS=%s"),
        (int32)bWasSuccessful, *InSessionName.ToString(), *GetOSSNameSafe());

    if (!bWasSuccessful)
    {
        OnSessionOpFailed.Broadcast(TEXT("CreateSession failed"));
        return;
    }

    // ✅ 1) 먼저 리슨 트래블 (호스트가 접속 받을 준비부터)
    FString URL = LobbyMapPath;
    if (!URL.Contains(TEXT("?listen")))
        URL += TEXT("?listen");

    UWorld* World = GetWorld(); // GI->GetWorld()보다 안정적
    if (!World)
    {
        OnSessionOpFailed.Broadcast(TEXT("No World for ServerTravel"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[ServerTravel] -> %s"), *URL);
    World->ServerTravel(URL);

    // ✅ 2) StartSession은 약간 뒤(또는 로비 GameMode BeginPlay 등에서)
    // 여기서는 간단히 타이머 예시
    StartHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(
        FOnStartSessionCompleteDelegate::CreateUObject(this, &UThe_Endless_Rooms_MatchSubsystem::OnStartSessionComplete));

    World->GetTimerManager().SetTimerForNextTick([this]()
        {
            if (SessionInterface.IsValid())
            {
                const bool bStarted = SessionInterface->StartSession(SessionName);
                if (!bStarted)
                {
                    OnSessionOpFailed.Broadcast(TEXT("StartSession() returned false"));
                    ClearDelegates();
                }
            }
        });
}

void UThe_Endless_Rooms_MatchSubsystem::OnStartSessionComplete(FName InSessionName, bool bWasSuccessful)
{
    ClearDelegates();

    UE_LOG(LogTemp, Warning, TEXT("[StartComplete] ok=%d InSessionName=%s OSS=%s"),
        (int32)bWasSuccessful, *InSessionName.ToString(), *GetOSSNameSafe());

    if (!bWasSuccessful)
    {
        OnSessionOpFailed.Broadcast(TEXT("StartSession failed"));
        return;
    }

    // ✅ 여기서는 이제 UI 갱신 같은 것만
    OnSessionCreated.Broadcast();
}

void UThe_Endless_Rooms_MatchSubsystem::FindSessions(int32 MaxResults, bool bIsLAN)
{
    EnsureSessionInterface();
    if (!SessionInterface.IsValid())
    {
        OnSessionOpFailed.Broadcast(TEXT("No SessionInterface (FindSessions)"));
        return;
    }

    bPendingIsLAN = bIsLAN;

    FUniqueNetIdRepl NetId;
    FString FailReason;
    if (!GetLocalPlayerNetId(NetId, FailReason))
    {
        OnSessionOpFailed.Broadcast(FString::Printf(TEXT("FindSessions: %s"), *FailReason));
        return;
    }

    SessionSearch = MakeShared<FOnlineSessionSearch>();

    // ✅ 실수 방지: Steam 온라인이면 LanQuery 강제 OFF
    const bool bOnline = !bIsLAN;
    SessionSearch->bIsLanQuery = bIsLAN;
    SessionSearch->MaxSearchResults = MaxResults;

    VisibleResultIndices.Reset();

    UE_LOG(LogTemp, Warning, TEXT("[FindSessions] OSS=%s LAN=%d Online=%d Max=%d NetId=%s Type=%s"),
        *GetOSSNameSafe(), (int32)bIsLAN, (int32)bOnline, MaxResults,
        *NetId->ToString(), *NetId->GetType().ToString());

    if (bOnline)
    {
        // ✅ Presence 기반
        SessionSearch->QuerySettings.Set(PRESENCE_KEY, true, EOnlineComparisonOp::Equals);

        // ✅ 로비 기반(유지)
        SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

        // ✅ (추가) Steam 로비 검색에서 잘 먹는 키워드 필터
        SessionSearch->QuerySettings.Set(SEARCH_KEYWORDS, FString(TEXT("THEENDLESSROOMS")), EOnlineComparisonOp::Equals);

        // ✅ 프로젝트/빌드 필터(OSS가 무시할 수도 있어 로컬 필터도 유지)
        SessionSearch->QuerySettings.Set(PROJECT_KEY, FString(TEXT("THEENDLESSROOMS")), EOnlineComparisonOp::Equals);
        SessionSearch->QuerySettings.Set(BUILD_ID_KEY, BACKROOM_BUILD_ID, EOnlineComparisonOp::Equals);

        SessionSearch->MaxSearchResults = FMath::Max(SessionSearch->MaxSearchResults, 500);
    }

    FindHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
        FOnFindSessionsCompleteDelegate::CreateUObject(this, &UThe_Endless_Rooms_MatchSubsystem::OnFindSessionsComplete));

    const bool bStarted = SessionInterface->FindSessions(*NetId, SessionSearch.ToSharedRef());
    if (!bStarted)
    {
        OnSessionOpFailed.Broadcast(TEXT("FindSessions() returned false"));
        ClearDelegates();
        return;
    }
}

void UThe_Endless_Rooms_MatchSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    ClearDelegates();

    if (!bWasSuccessful || !SessionSearch.IsValid())
    {
        OnSessionOpFailed.Broadcast(TEXT("FindSessions failed"));
        return;
    }

    const int32 RawCount = SessionSearch->SearchResults.Num();

    TArray<FTheEndlessRoomsSessionInfo> Out;
    VisibleResultIndices.Reset();

    int32 FilteredOutByTag = 0;

    for (int32 i = 0; i < RawCount; ++i)
    {
        const FOnlineSessionSearchResult& R = SessionSearch->SearchResults[i];

        FString ProjectTag(TEXT("NONE"));
        const bool bHasProjectTag = R.Session.SessionSettings.Get(PROJECT_KEY, ProjectTag);

        int32 BuildId = -1;
        const bool bHasBuildId = R.Session.SessionSettings.Get(BUILD_ID_KEY, BuildId);

        UE_LOG(LogTemp, Warning, TEXT("[FindDump] idx=%d Host=%s Ping=%d HasTag=%d Tag=%s HasBuild=%d BuildId=%d OpenPub=%d/%d"),
            i,
            *R.Session.OwningUserName,
            R.PingInMs,
            (int32)bHasProjectTag,
            *ProjectTag,
            (int32)bHasBuildId,
            BuildId,
            R.Session.NumOpenPublicConnections,
            R.Session.SessionSettings.NumPublicConnections);

        // ✅ 온라인일 때만 강제 필터
        if (!bPendingIsLAN)
        {
            const bool bTagOk = (bHasProjectTag && ProjectTag == TEXT("THEENDLESSROOMS"));
            const bool bBuildOk = (bHasBuildId && BuildId == BACKROOM_BUILD_ID);

            if (!bTagOk || !bBuildOk)
            {
                ++FilteredOutByTag;
                continue;
            }
        }

        FTheEndlessRoomsSessionInfo Info;
        Info.PingMs = R.PingInMs;

        const FOnlineSession& S = R.Session;
        Info.MaxPlayers = S.SessionSettings.NumPublicConnections;
        Info.CurrentPlayers = Info.MaxPlayers - S.NumOpenPublicConnections;
        Info.HostName = S.OwningUserName;

        Out.Add(Info);
        VisibleResultIndices.Add(i);
    }

    OnSessionsFound.Broadcast(Out);

    UE_LOG(LogTemp, Warning, TEXT("[FindComplete] Success=%d RawResults=%d FilteredResults=%d FilteredOut=%d OSS=%s"),
        (int32)bWasSuccessful, RawCount, Out.Num(), FilteredOutByTag, *GetOSSNameSafe());
}

void UThe_Endless_Rooms_MatchSubsystem::JoinSessionByIndex(int32 Index)
{
    EnsureSessionInterface();
    if (!SessionInterface.IsValid() || !SessionSearch.IsValid())
    {
        OnSessionOpFailed.Broadcast(TEXT("No SessionSearch/Interface"));
        return;
    }

    if (!VisibleResultIndices.IsValidIndex(Index))
    {
        OnSessionOpFailed.Broadcast(TEXT("Invalid filtered session index"));
        return;
    }

    const int32 RealIndex = VisibleResultIndices[Index];
    if (!SessionSearch->SearchResults.IsValidIndex(RealIndex))
    {
        OnSessionOpFailed.Broadcast(TEXT("Invalid real session index"));
        return;
    }

    FUniqueNetIdRepl NetId;
    FString FailReason;
    if (!GetLocalPlayerNetId(NetId, FailReason))
    {
        OnSessionOpFailed.Broadcast(FString::Printf(TEXT("JoinSession: %s"), *FailReason));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[JoinSessionByIndex] UIIndex=%d RealIndex=%d Host=%s OSS=%s"),
        Index, RealIndex, *SessionSearch->SearchResults[RealIndex].Session.OwningUserName, *GetOSSNameSafe());

    JoinHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
        FOnJoinSessionCompleteDelegate::CreateUObject(this, &UThe_Endless_Rooms_MatchSubsystem::OnJoinSessionComplete));

    const bool bStarted = SessionInterface->JoinSession(
        *NetId,
        SessionName,
        SessionSearch->SearchResults[RealIndex]
    );

    if (!bStarted)
    {
        OnSessionOpFailed.Broadcast(TEXT("JoinSession() returned false"));
        ClearDelegates();
        return;
    }
}

void UThe_Endless_Rooms_MatchSubsystem::OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result)
{
    ClearDelegates();

    const FString ResultStr = JoinResultToString(Result);
    UE_LOG(LogTemp, Warning, TEXT("[JoinComplete] Result=%s(%d) InSessionName=%s OSS=%s"),
        *ResultStr, (int32)Result, *InSessionName.ToString(), *GetOSSNameSafe());

    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        OnSessionOpFailed.Broadcast(FString::Printf(TEXT("JoinSession failed: %s"), *ResultStr));
        return;
    }

    FString ConnectString;
    bool bHasAddr = false;

    if (SessionInterface.IsValid())
    {
        bHasAddr = SessionInterface->GetResolvedConnectString(InSessionName, ConnectString);

        if (!bHasAddr)
        {
            bHasAddr = SessionInterface->GetResolvedConnectString(InSessionName, ConnectString, NAME_GamePort);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[JoinComplete] ConnectString='%s' (ok=%d)"),
        *ConnectString, (int32)bHasAddr);

    if (!bHasAddr || ConnectString.IsEmpty())
    {
        OnSessionOpFailed.Broadcast(TEXT("Join succeeded but no connect string (CouldNotRetrieveAddress)"));
        return;
    }

    OnSessionJoined.Broadcast();

    UGameInstance* GI = GetGameInstance();
    UWorld* World = GI ? GI->GetWorld() : nullptr;
    if (!World)
    {
        OnSessionOpFailed.Broadcast(TEXT("No World after join"));
        return;
    }

    if (APlayerController* PC = World->GetFirstPlayerController())
    {
        UE_LOG(LogTemp, Warning, TEXT("[ClientTravel] -> %s"), *ConnectString);
        PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
    }
    else
    {
        OnSessionOpFailed.Broadcast(TEXT("No PlayerController"));
    }
}

void UThe_Endless_Rooms_MatchSubsystem::DestroySession()
{
    EnsureSessionInterface();
    if (!SessionInterface.IsValid())
    {
        OnSessionOpFailed.Broadcast(TEXT("No SessionInterface (DestroySession)"));
        return;
    }

    if (!SessionInterface->GetNamedSession(SessionName))
    {
        // 세션이 없으면 “재호스트 대기”가 걸려있더라도 바로 Host로 이어갈 수도 있음
        if (bRecreateAfterDestroy)
        {
            bRecreateAfterDestroy = false;
            HostSession(PendingMaxPlayers, bPendingIsLAN);
        }
        return;
    }

    DestroyHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
        FOnDestroySessionCompleteDelegate::CreateUObject(this, &UThe_Endless_Rooms_MatchSubsystem::OnDestroySessionComplete));

    const bool bStarted = SessionInterface->DestroySession(SessionName);
    if (!bStarted)
    {
        OnSessionOpFailed.Broadcast(TEXT("DestroySession() returned false"));
        ClearDelegates();
        return;
    }
}

void UThe_Endless_Rooms_MatchSubsystem::OnDestroySessionComplete(FName InSessionName, bool bWasSuccessful)
{
    ClearDelegates();

    UE_LOG(LogTemp, Warning, TEXT("[DestroyComplete] ok=%d name=%s OSS=%s"),
        (int32)bWasSuccessful, *InSessionName.ToString(), *GetOSSNameSafe());

    if (!bWasSuccessful)
    {
        OnSessionOpFailed.Broadcast(TEXT("DestroySession failed"));
        bRecreateAfterDestroy = false;
        return;
    }

    if (bRecreateAfterDestroy)
    {
        bRecreateAfterDestroy = false;
        HostSession(PendingMaxPlayers, bPendingIsLAN);
    }
}

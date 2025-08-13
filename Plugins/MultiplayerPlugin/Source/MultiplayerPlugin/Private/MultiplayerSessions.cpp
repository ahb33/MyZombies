// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerSessions.h"
#include "OnlineSubsystem.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Engine/Engine.h"                     // For GEngine
#include "Engine/World.h"                      // For UWorld
#include "GameFramework/PlayerController.h"    // For APlayerController
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/LocalPlayer.h"                // For ULocalPlayer
#include "Engine/GameViewportClient.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"    
#include "Engine/EngineTypes.h"                // For ETravelType / TRAVEL_Absolute


UMultiplayerSessions::UMultiplayerSessions()
    : CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
      FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionComplete)),
      JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
      DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
      StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
    // get online subsystem and store in variable
    // check if subsystem variable is valid
    // get session interface from subsystem variable and store in another variable

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("UMultiplayerSessions Constructor Called"));
    }
}

void UMultiplayerSessions::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Initialize: No OnlineSubsystem found"));
        return;
    }

    sessionInterface = Subsystem->GetSessionInterface();
    if (!sessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Initialize: GetSessionInterface() failed"));
        return;
    }

    const FName Name = Subsystem->GetSubsystemName();
    UE_LOG(LogTemp, Display, TEXT("Initialize: Using subsystem %s"), *Name.ToString());
}
void UMultiplayerSessions::Deinitialize()
{
    if (sessionInterface.IsValid())
    {
        sessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
        sessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
        sessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
        sessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
        sessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
    }
    Super::Deinitialize();
}

/*
    The function first checks if a session already exists and destroys it if needed to ensure we start with a clean slate.
    sessionSettings are configured based on variables OnlinesessionSettings.h
    then we call sessionInterface's create session function
    finally, CreateSessionCompleteDelegate is bound to handle the session creation result, which will trigger the OnCreateSessionComplete callback.



*/
void UMultiplayerSessions::CreateSession(int32 NumPublicConnections, const FString& MatchType)
{
    UE_LOG(LogTemp, Warning, TEXT("Create Session Called"));
    
    if (!sessionInterface.IsValid())
    {
        MultiplayerOnCreateSessionComplete.Broadcast(false);
        return;
    }

    // If a session already exists, destroy it first.
    auto ExistingSession = sessionInterface->GetNamedSession(GameSessionName);
    if (ExistingSession != nullptr)
    {
        sessionInterface->DestroySession(GameSessionName);
    }

    // Initialize session settings
    sessionSettings = MakeShared<FOnlineSessionSettings>();

    const bool bIsLAN = Subsystem->GetSubsystemName() == "NULL";

    sessionSettings->bIsLANMatch = bIsLAN;
    sessionSettings->NumPublicConnections = NumPublicConnections;
    CachedPublicConnections = NumPublicConnections;
    sessionSettings->bShouldAdvertise = true;
    sessionSettings->bAllowJoinInProgress = true;
    sessionSettings->bAllowJoinViaPresence = false;
    sessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
    sessionSettings->bUsesPresence = false; // ❌ OFF if LAN
    sessionSettings->bUseLobbiesIfAvailable = false; // ❌ NO LOBBIES IN LAN
    sessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineService);
    
    if (bIsLAN)
    {
        FString LocalIP;
        bool bGotIP = false;

        TSharedRef<FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bGotIP);
        if (bGotIP)
        {
            LocalIP = Addr->ToString(false); 
            sessionSettings->Set(FName("HostIP"), LocalIP, EOnlineDataAdvertisementType::ViaOnlineService);
            UE_LOG(LogTemp, Warning, TEXT("Advertising LAN session with host IP: %s"), *LocalIP);
        }
    }

    // Set custom keys
    const FName MATCHING_TIMEOUT_KEY = TEXT("MatchingTimeout");
    sessionSettings->Set(MATCHING_TIMEOUT_KEY, 120.0f, EOnlineDataAdvertisementType::ViaOnlineService);

    // Set match type in destination projects - 
    const FName MATCH_TYPE_KEY = TEXT("MatchType");
    sessionSettings->Set(MATCH_TYPE_KEY, MatchType, EOnlineDataAdvertisementType::ViaOnlineService);

    // Bind the delegate and store the handle.
    CreateSessionCompleteDelegateHandle = sessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
 
    // Create the session with the session settings
    auto LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LocalPlayer)
    {
        MultiplayerOnCreateSessionComplete.Broadcast(false);
        return;
    }

    // Attempt to create the session.
    if (!sessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), GameSessionName, *sessionSettings))
    {
        sessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
        UE_LOG(LogTemp, Warning, TEXT("Failed to create session!"));
        MultiplayerOnCreateSessionComplete.Broadcast(false);
        return;
    }
}

void UMultiplayerSessions::FindSessions(APlayerController* PlayerController, int32 MaxSearchResults, const FString& MatchType)
{
    if (!sessionInterface.IsValid())
    {
        MultiplayerOnFindSessionCompleteBP.Broadcast({}, false); // Use new Blueprint-safe delegate
        return;
    }

    searchSettings = MakeShared<FOnlineSessionSearch>();
    searchSettings->MaxSearchResults = MaxSearchResults;
    searchSettings->bIsLanQuery = (Subsystem->GetSubsystemName() == NullSubsystemName);
    searchSettings->QuerySettings.Set(FName("SEARCH_PRESENCE"), true, EOnlineComparisonOp::Equals);
    searchSettings->QuerySettings.Set(TEXT("MatchType"), MatchType, EOnlineComparisonOp::Equals);

    FindSessionsCompleteDelegateHandle = sessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

    ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
    sessionInterface->FindSessions(LocalPlayer->GetControllerId(), searchSettings.ToSharedRef());
}

void UMultiplayerSessions::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
    UE_LOG(LogTemp, Warning, TEXT("JoinSession called"));

    if (!sessionInterface.IsValid())
    {
        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
        return;
    }

    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LocalPlayer || !LocalPlayer->GetPreferredUniqueNetId().IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Invalid LocalPlayer or NetId"));
        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
        return;
    }

    JoinSessionCompleteDelegateHandle = sessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

    if (!sessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
    {
        sessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
    }
}

void UMultiplayerSessions::DestroySession()
{
    UE_LOG(LogTemp, Warning, TEXT("DestroySession called"));

    if (!sessionInterface.IsValid())
    {
        MultiplayerOnDestroySessionComplete.Broadcast(false);
        return;
    }

    // Bind the delegate to listen for completion
    DestroySessionCompleteDelegateHandle = sessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
    FOnDestroySessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessions::OnDestroySessionComplete));

    // Attempt to destroy the session
    if (!sessionInterface->DestroySession(GameSessionName))
    {
        sessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
        MultiplayerOnDestroySessionComplete.Broadcast(false);
    }
}


void UMultiplayerSessions::StartSession()
{
    UE_LOG(LogTemp, Warning, TEXT("StartSession called"));

    if (!sessionInterface.IsValid())
    {
        MultiplayerOnStartSessionComplete.Broadcast(false);
        return;
    }

    // Bind the delegate to listen for completion
    StartSessionCompleteDelegateHandle = sessionInterface->AddOnStartSessionCompleteDelegate_Handle(
        FOnStartSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessions::OnStartSessionComplete));

    // Attempt to start the session
    if (!sessionInterface->StartSession(GameSessionName))
    {
        sessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
        MultiplayerOnStartSessionComplete.Broadcast(false);
        UE_LOG(LogTemp, Warning, TEXT("Failed to start session!"));
    }
}
void UMultiplayerSessions::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionComplete called. Success: %s"), bWasSuccessful ? TEXT("true") : TEXT("false"));

    if (sessionInterface.IsValid())
    {
        sessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
    }

    if (bWasSuccessful)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            FString LobbyMap = TEXT("/Game/GameAssets/Levels/LobbyLevel?listen");
            World->ServerTravel(LobbyMap);
        }
    }
    
    MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);

}

void UMultiplayerSessions::OnFindSessionComplete(bool bWasSuccessful)
{
    if (sessionInterface.IsValid())
    {
        sessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
    }

    if (!sessionInterface.IsValid() || !searchSettings.IsValid())
    {
        MultiplayerOnFindSessionCompleteBP.Broadcast({}, false);
        return;
    }
    
    LastSearchResults = searchSettings->SearchResults;

    UE_LOG(LogTemp, Warning, TEXT("Raw Results Found: %d"), LastSearchResults.Num());

#if WITH_EDITOR
    for (const auto& Raw : LastSearchResults)
    {
        FString MatchType;
        Raw.Session.SessionSettings.Get(FName("MatchType"), MatchType);
        UE_LOG(LogTemp, Warning, TEXT("Owner of game: %s | MatchType: %s"), *Raw.Session.OwningUserName, *MatchType);
    }
#endif

    UE_LOG(LogTemp, Warning, TEXT("Raw Results Found: %d"), LastSearchResults.Num());

    for (const auto& Raw : LastSearchResults)
    {
        FString Match;
        Raw.Session.SessionSettings.Get(TEXT("MatchType"), Match);
        FString Owning = Raw.Session.OwningUserName;
        int32 Open = Raw.Session.NumOpenPublicConnections;
        int32 Max = Raw.Session.SessionSettings.NumPublicConnections;
        
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Found Session: %s (%s) %d/%d"), *Owning, *Match, Max - Open, Max));
    }
    MultiplayerOnFindSessionCompleteBP.Broadcast(GetBlueprintSearchResults(), bWasSuccessful);
}

void UMultiplayerSessions::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionComplete: %d"), (int32)Result);

    if (sessionInterface)
    {
        sessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
    }

    FString ConnectString;
    if (!sessionInterface->GetResolvedConnectString(SessionName, ConnectString))
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Failed to get connect string."));
        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Connect string resolved: %s"), *ConnectString);

    UWorld* World = GetWorld();
    if (APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr)
    {
        PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
    }

    MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessions::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Warning, TEXT("OnDestroySessionComplete called with SessionName = %s, bWasSuccessful = %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

    if (sessionInterface.IsValid())
    {
        sessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
    }
    
    MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
    
    if (bHasPendingJoin)
    {
        bHasPendingJoin = false;
    }

    UE_LOG(LogTemp, Warning, TEXT("Session successfully destroyed. Ready for a new session if needed."));
}

void UMultiplayerSessions::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Warning, TEXT("OnStartSessionComplete called with SessionName = %s, bWasSuccessful = %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

    if (sessionInterface.IsValid())
    {
        sessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
    }

    MultiplayerOnStartSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessions::JoinSessionByIndex(int32 SessionIndex)
{
    if (LastSearchResults.IsValidIndex(SessionIndex))
    {
        JoinSession(LastSearchResults[SessionIndex]);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("JoinSessionByIndex: invalid index %d"), SessionIndex);
    }
}

TArray<FMySessionResult> UMultiplayerSessions::GetBlueprintSearchResults() const
{
    TArray<FMySessionResult> Results;

    for (const auto& Raw : LastSearchResults)
    {
        FMySessionResult Entry;
        Entry.OwningUserName = Raw.Session.OwningUserName;
        Entry.PingInMs       = Raw.PingInMs;
        Entry.MaxPlayers     = Raw.Session.SessionSettings.NumPublicConnections;
        Entry.CurrentPlayers = Entry.MaxPlayers - Raw.Session.NumOpenPublicConnections;
        Entry.RawResult      = Raw; // ✅ crucial for Join
        Results.Add(Entry);
    }

    return Results;
}


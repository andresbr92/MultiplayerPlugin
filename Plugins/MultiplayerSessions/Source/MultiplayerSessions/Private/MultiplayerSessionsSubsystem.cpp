// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Online/OnlineSessionNames.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionComplete)),
JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld());
	if (Subsystem)
	{
		SessionInterface = Subsystem->GetSessionInterface();
	}
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 MaxPublicConnections, FString MatchType, FString Path)
{
	if (!SessionInterface.IsValid())
		return;
	PathToTravel = Path;
	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = MaxPublicConnections;
		LastMatchType = MatchType;
		DestroySession();
	}

    CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
	// CREATE SESSION SETTINGS
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = Online::GetSubsystem(GetWorld())->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSettings->NumPublicConnections = MaxPublicConnections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	
	// CREATE SESSION
	bool bCreateSessionSuccess = SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings);
	if (!bCreateSessionSuccess)
	{
		// Remove the delegate handle if the session creation failed
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	} else
	{
		// Broadcast the event if the session was created successfully
		MultiplayerOnCreateSessionComplete.Broadcast(true);
	}
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}
	FindSessionCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	SessionSearch = MakeShareable(new FOnlineSessionSearch);
	SessionSearch->MaxSearchResults = MaxSearchResults;
	SessionSearch->bIsLanQuery = Online::GetSubsystem(GetWorld())->GetSubsystemName() == "NULL" ? true : false;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	bool bFindSessionSuccess = SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(),
	                                                          SessionSearch.ToSharedRef());
	if (!bFindSessionSuccess)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegateHandle);
		MultiplayerOnFindSessionsComplete.Broadcast(FOnlineSessionSearchCustom(), false);
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(FSessionInfo SessionInfo)
{
	if (!SessionInterface.IsValid()) return;
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
	for (auto DesiredSession: SessionSearch->SearchResults)
	{
		FString Id = DesiredSession.GetSessionIdStr();
		FString User = DesiredSession.Session.OwningUserName;
		FString MatchType;
		DesiredSession.Session.SessionSettings.Get(FName("MatchType"), MatchType);
		if (MatchType == SessionInfo.MatchType)
		{
			
			UE_LOG(LogTemp, Warning, TEXT("Joining Match Type: %s"), *MatchType);
			DesiredSession.Session.SessionSettings.bUseLobbiesIfAvailable = true;

			const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
			bool bJoinSessionSuccess = SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, DesiredSession);
			if (!bJoinSessionSuccess)
			{
				SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
				MultiplayerOnJoinSessionsComplete.Broadcast(SessionInfo, false);
			}
		}
	}
}


void UMultiplayerSessionsSubsystem::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}
	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
	bool bDestroySessionSuccess = SessionInterface->DestroySession(NAME_GameSession);
	if (!bDestroySessionSuccess)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
	
}

void UMultiplayerSessionsSubsystem::StartSession()
{
	SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->ServerTravel(PathToTravel);
		
	}
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionComplete(bool bWasSuccessful)
{
	
	TArray<FSessionInfo> SessionInfos;
	if (bWasSuccessful)
	{
		for (auto Result: SessionSearch->SearchResults)
		{
			FSessionInfo SessionInfo;
			SessionInfo.SessionId = Result.GetSessionIdStr();
			SessionInfo.OwningUserName = Result.Session.OwningUserName;
			FString MatchType;
			Result.Session.SessionSettings.Get(FName("MatchType"), MatchType);
			SessionInfo.MatchType = MatchType;
			SessionInfos.Add(SessionInfo);
		}
	}
	FOnlineSessionSearchCustom SearchResults;
	SearchResults.SearchResults = SessionInfos;


	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegateHandle);
	MultiplayerOnFindSessionsComplete.Broadcast(SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName,
	EOnJoinSessionCompleteResult::Type Result)
{
	if (!SessionInterface.IsValid()) return;
	// Access de IP address to travel to
	FString Address;

	if (SessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
	{
		
		UE_LOG(LogTemp, Warning, TEXT("The IP address is: %s"), *Address);
		APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
		if (PlayerController)
		{
			PlayerController->ClientTravel(Address, TRAVEL_Absolute);
		}
	}
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	MultiplayerOnJoinSessionsComplete.Broadcast(FSessionInfo(), Result == EOnJoinSessionCompleteResult::Success);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastMatchType, PathToTravel);
	}
	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

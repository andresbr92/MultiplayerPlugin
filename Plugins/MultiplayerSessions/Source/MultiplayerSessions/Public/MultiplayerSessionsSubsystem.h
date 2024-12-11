// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MultiplayerSessionsSubsystem.generated.h"

/**
 *  Declaring our own custom delegates for components that uses this subsystem
 */
USTRUCT()
struct FOnlineSessionSearchCustom
{
	GENERATED_BODY()
	TArray<FOnlineSessionSearchResult> SearchResults;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, FOnlineSessionSearchCustom, Result, bool, bWasSuccessful);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);


UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UMultiplayerSessionsSubsystem();
	UFUNCTION(BlueprintCallable)
	void CreateSession(int32 MaxPublicConnections, FString MatchType);
	UFUNCTION(BlueprintCallable)
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	UFUNCTION(BlueprintCallable)
	void DestroySession();
	UFUNCTION(BlueprintCallable)
	void StartSession();
	/**
	 *  Our own custom delegates for the components to bind callbacks
	 */
	UPROPERTY(BlueprintAssignable)
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	UPROPERTY(BlueprintAssignable)
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	// FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionsComplete;
	UPROPERTY(BlueprintAssignable)
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	UPROPERTY(BlueprintAssignable)
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;
protected:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	//
	// To add the Online Session Interface delegate list.
	// We'll bind our MultiplayerSessionSubsystem internal callbacks to these
	//
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionCompleteDelegateHandle;
	
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;
	// session search
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
};

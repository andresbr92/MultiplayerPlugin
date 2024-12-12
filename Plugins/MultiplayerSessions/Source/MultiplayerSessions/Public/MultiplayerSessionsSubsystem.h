// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MultiplayerSessionsSubsystem.generated.h"
USTRUCT(BlueprintType)
struct FSessionInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FString MatchType;
	UPROPERTY(BlueprintReadOnly)
	FString SessionId;

	UPROPERTY(BlueprintReadOnly)
	FString OwningUserName;

	UPROPERTY(BlueprintReadOnly)
	int32 NumOpenPublicConnections;

	UPROPERTY(BlueprintReadOnly)
	int32 PingInMs;
};
/**
 *  Declaring our own custom delegates for components that uses this subsystem
 */
USTRUCT(BlueprintType)
struct FOnlineSessionSearchCustom
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	TArray<FSessionInfo> SearchResults;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, FOnlineSessionSearchCustom, Result, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnJoinSessionComplete, FSessionInfo, SessionInfo, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);


UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UMultiplayerSessionsSubsystem();
	UFUNCTION(BlueprintCallable)
	void CreateSession(int32 MaxPublicConnections, FString MatchType, FString Path);
	UFUNCTION(BlueprintCallable)
	void FindSessions(int32 MaxSearchResults);
	UFUNCTION(BlueprintCallable)
	void JoinSession(FSessionInfo SessionInfo);
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
	UPROPERTY(BlueprintAssignable)
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionsComplete;
	UPROPERTY(BlueprintAssignable)
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	UPROPERTY(BlueprintAssignable)
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;
protected:
	// Bound functions
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
private:
	// Main interface
	IOnlineSessionPtr SessionInterface;
	// Session Settings
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	// Session Search
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	// Path to trave
	FString PathToTravel{FString("")};
	
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

};

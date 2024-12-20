// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (GameState)
	{
		int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
		
		UE_LOG(LogTemp, Warning, TEXT("Number of players: %d"), NumberOfPlayers);
		// access to the player state
		APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
		if (PlayerState)
		{
			FString PlayerName = PlayerState->GetPlayerName();
			UE_LOG(LogTemp, Warning, TEXT("Player name: %s"), *PlayerName);
		}
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	if (GameState)
	{
		int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
		
		UE_LOG(LogTemp, Warning, TEXT("Number of players: %d"), NumberOfPlayers - 1);
	}
	APlayerState* PlayerState = Exiting->GetPlayerState<APlayerState>();
	if (PlayerState)
	{
		FString PlayerName = PlayerState->GetPlayerName();
		
		UE_LOG(LogTemp, Warning, TEXT("Player name: %s"), *PlayerName);
	}
}

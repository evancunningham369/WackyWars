// Fill out your copyright notice in the Description page of Project Settings. 


#include "WackyGameMode.h"
#include "WackyWars/Character/WackyCharacter.h"
#include "WackyWars/PlayerController/WackyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "WackyWars/PlayerState/WackyPlayerState.h"
#include "WackyWars/GameState/WackyGameState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

AWackyGameMode::AWackyGameMode()
{
	bDelayedStart = true;
}

void AWackyGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}


void AWackyGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void AWackyGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AWackyPlayerController* WackyPlayer = Cast<AWackyPlayerController>(*It);
		if (WackyPlayer)
		{
			WackyPlayer->OnMatchStateSet(MatchState, bTeamsMatch);
		}
	}
}

float AWackyGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}

void AWackyGameMode::PlayerEliminated(AWackyCharacter* ElimmedCharacter, AWackyPlayerController* VictimController, AWackyPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;

	AWackyPlayerState* AttackerPlayerState = AttackerController ? Cast<AWackyPlayerState>(AttackerController->PlayerState) : nullptr;
	AWackyPlayerState* VictimPlayerState = VictimController ? Cast<AWackyPlayerState>(VictimController->PlayerState) : nullptr;

	AWackyGameState* WackyGameState = GetGameState<AWackyGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && WackyGameState)
	{
		TArray<AWackyPlayerState*> PlayersCurrentlyInTheLead;
		for (auto LeadPlayer : WackyGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}

		AttackerPlayerState->AddToScore(1.f);
		WackyGameState->UpdateTopScore(AttackerPlayerState);
		if (WackyGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			AWackyCharacter* Leader = Cast<AWackyCharacter>(AttackerPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}

		for (int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{
			if (!WackyGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
				AWackyCharacter* Loser = Cast<AWackyCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn());
				if (Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AWackyPlayerController* WackyPlayer = Cast<AWackyPlayerController>(*It);
		if (WackyPlayer && AttackerPlayerState && VictimPlayerState)
		{
			WackyPlayer->BrodcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}

void AWackyGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

void AWackyGameMode::PlayerLeftGame(AWackyPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	AWackyGameState* WackyGameState = GetGameState<AWackyGameState>();
	if (WackyGameState && WackyGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		WackyGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	AWackyCharacter* CharacterLeaving = Cast<AWackyCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}
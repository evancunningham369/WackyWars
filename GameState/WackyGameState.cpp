// Fill out your copyright notice in the Description page of Project Settings. 


#include "WackyGameState.h"
#include "Net/UnrealNetwork.h"
#include "WackyWars/PlayerState/WackyPlayerState.h"
#include "WackyWars/PlayerController/WackyPlayerController.h"

void AWackyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWackyGameState, TopScoringPlayers);
	DOREPLIFETIME(AWackyGameState, RedTeamScore);
	DOREPLIFETIME(AWackyGameState, BlueTeamScore);
}

void AWackyGameState::UpdateTopScore(AWackyPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void AWackyGameState::RedTeamScores()
{
	++RedTeamScore;
	
	AWackyPlayerController* WPlayer = Cast<AWackyPlayerController>(GetWorld()->GetFirstPlayerController());
	if (WPlayer)
	{
		WPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AWackyGameState::BlueTeamScores()
{
	++BlueTeamScore;

	AWackyPlayerController* WPlayer = Cast<AWackyPlayerController>(GetWorld()->GetFirstPlayerController());
	if (WPlayer)
	{
		WPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void AWackyGameState::OnRep_RedTeamScore()
{
	AWackyPlayerController* WPlayer = Cast<AWackyPlayerController>(GetWorld()->GetFirstPlayerController());
	if (WPlayer)
	{
		WPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AWackyGameState::OnRep_BlueTeamScore()
{
	AWackyPlayerController* WPlayer = Cast<AWackyPlayerController>(GetWorld()->GetFirstPlayerController());
	if (WPlayer)
	{
		WPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

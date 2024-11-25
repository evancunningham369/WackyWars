// Fill out your copyright notice in the Description page of Project Settings. 


#include "CaptureTheFlagGameMode.h"
#include "WackyWars/Weapon/Flag.h"
#include "WackyWars/CaptureTheFlag/FlagZone.h"
#include "WackyWars/GameState/WackyGameState.h"

void ACaptureTheFlagGameMode::PlayerEliminated(AWackyCharacter* ElimmedCharacter, AWackyPlayerController* VictimController, AWackyPlayerController* AttackerController)
{
	AWackyGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
}

void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
	bool bValidCapture = Flag->GetTeam() != Zone->Team;
	AWackyGameState* WGameState = Cast<AWackyGameState>(GameState);
	if (WGameState)
	{
		if (Zone->Team == ETeam::ET_BlueTeam)
		{
			WGameState->BlueTeamScores();
		}
		if (Zone->Team == ETeam::ET_RedTeam)
		{
			WGameState->RedTeamScores();
		}
	}
}

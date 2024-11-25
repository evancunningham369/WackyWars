// Fill out your copyright notice in the Description page of Project Settings. 


#include "TeamsGameMode.h"
#include "WackyWars/GameState/WackyGameState.h"
#include "WackyWars/PlayerState/WackyPlayerState.h"
#include "WackyWars/PlayerController/WackyPlayerController.h"
#include "Kismet/GameplayStatics.h"

ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AWackyGameState* WGameState = Cast<AWackyGameState>(UGameplayStatics::GetGameState(this));
	if (WGameState)
	{
		AWackyPlayerState* WPState = NewPlayer->GetPlayerState<AWackyPlayerState>();
		if (WPState && WPState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (WGameState->BlueTeam.Num() >= WGameState->RedTeam.Num())
			{
				WGameState->RedTeam.AddUnique(WPState);
				WPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				WGameState->BlueTeam.AddUnique(WPState);
				WPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	AWackyGameState* WGameState = Cast<AWackyGameState>(UGameplayStatics::GetGameState(this));
	AWackyPlayerState* WPState = Exiting->GetPlayerState<AWackyPlayerState>();
	if (WGameState && WPState)
	{
		if (WGameState->RedTeam.Contains(WPState))
		{
			WGameState->RedTeam.Remove(WPState);
		}
		if (WGameState->BlueTeam.Contains(WPState))
		{
			WGameState->BlueTeam.Remove(WPState);
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	AWackyPlayerState* AttackerPState = Attacker->GetPlayerState<AWackyPlayerState>();
	AWackyPlayerState* VictimPState = Victim->GetPlayerState<AWackyPlayerState>();
	if (AttackerPState == nullptr || VictimPState == nullptr) return BaseDamage;
	if (VictimPState == AttackerPState)
	{
		return BaseDamage;
	}
	if (AttackerPState->GetTeam() == VictimPState->GetTeam())
	{
		return 0.f;
	}
	return BaseDamage;
}

void ATeamsGameMode::PlayerEliminated(AWackyCharacter* ElimmedCharacter, AWackyPlayerController* VictimController, AWackyPlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	AWackyGameState* WGameState = Cast<AWackyGameState>(UGameplayStatics::GetGameState(this));
	AWackyPlayerState* AttackerPlayerState = AttackerController ? Cast<AWackyPlayerState>(AttackerController->PlayerState) : nullptr;
	if (WGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			WGameState->BlueTeamScores();
		}
		if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			WGameState->RedTeamScores();
		}
	}
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	AWackyGameState* WGameState = Cast<AWackyGameState>(UGameplayStatics::GetGameState(this));
	if (WGameState)
	{
		for (auto PState : WGameState->PlayerArray)
		{
			AWackyPlayerState* WPState = Cast<AWackyPlayerState>(PState.Get());
			if (WPState && WPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (WGameState->BlueTeam.Num() >= WGameState->RedTeam.Num())
				{
					WGameState->RedTeam.AddUnique(WPState);
					WPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					WGameState->BlueTeam.AddUnique(WPState);
					WPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

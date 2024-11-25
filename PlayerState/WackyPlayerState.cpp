// Fill out your copyright notice in the Description page of Project Settings.


#include "WackyPlayerState.h"
#include "WackyWars/Character/WackyCharacter.h"
#include "WackyWars/PlayerController/WackyPlayerController.h"
#include "Net/UnrealNetwork.h"

void AWackyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWackyPlayerState, Defeats);
	DOREPLIFETIME(AWackyPlayerState, Team);
}

void AWackyPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<AWackyCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AWackyPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AWackyPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<AWackyCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AWackyPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}


void AWackyPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<AWackyCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AWackyPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void AWackyPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;

	AWackyCharacter* WCharacter = Cast<AWackyCharacter>(GetPawn());
	if (WCharacter)
	{
		WCharacter->SetTeamColor(Team);
	}
}

void AWackyPlayerState::OnRep_Team()
{
	AWackyCharacter* WCharacter = Cast<AWackyCharacter>(GetPawn());
	if (WCharacter)
	{
		WCharacter->SetTeamColor(Team);
	}
}

void AWackyPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<AWackyCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AWackyPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

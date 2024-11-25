// Fill out your copyright notice in the Description page of Project Settings. 

#pragma once

#include "CoreMinimal.h"
#include "WackyGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class WACKYWARS_API ATeamsGameMode : public AWackyGameMode
{
	GENERATED_BODY()
public:
	ATeamsGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
	virtual void PlayerEliminated(class AWackyCharacter* ElimmedCharacter, class AWackyPlayerController* VictimController, class AWackyPlayerController* AttackerController) override;

protected:
	virtual void HandleMatchHasStarted() override;
};

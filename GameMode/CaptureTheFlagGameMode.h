// Fill out your copyright notice in the Description page of Project Settings. 

#pragma once

#include "CoreMinimal.h"
#include "TeamsGameMode.h"
#include "CaptureTheFlagGameMode.generated.h"

/**
 * 
 */
UCLASS()
class WACKYWARS_API ACaptureTheFlagGameMode : public ATeamsGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(class AWackyCharacter* ElimmedCharacter, class AWackyPlayerController* VictimController, class AWackyPlayerController* AttackerController) override;
	void FlagCaptured(class AFlag* Flag, class AFlagZone* Zone);
};

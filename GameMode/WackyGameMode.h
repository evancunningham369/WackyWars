// Fill out your copyright notice in the Description page of Project Settings. 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "WackyGameMode.generated.h"

namespace MatchState
{
	extern WACKYWARS_API const FName Cooldown; // Match duration has been reached. Display winner and begin cooldown timer
}

/**
 * 
 */
UCLASS()
class WACKYWARS_API AWackyGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AWackyGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class AWackyCharacter* ElimmedCharacter, class AWackyPlayerController* VictimController, class AWackyPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
	void PlayerLeftGame(class AWackyPlayerState* PlayerLeaving);
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;
	
	bool bTeamsMatch = false;
protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
private:
	float CountdownTime = 0.f;
public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};

// Fill out your copyright notice in the Description page of Project Settings. h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WackyWars/WackyTypes/Team.h"
#include "FlagZone.generated.h"

UCLASS()
class WACKYWARS_API AFlagZone : public AActor
{
	GENERATED_BODY()
	
public:	
	AFlagZone();

	UPROPERTY(EditAnywhere)
	ETeam Team;
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyInex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
private:

	UPROPERTY(EditAnywhere)
	class USphereComponent* ZoneSphere;
public:	
	

};

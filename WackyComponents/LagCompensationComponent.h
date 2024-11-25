// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

		UPROPERTY()
		bool bHitConfirmed;

	UPROPERTY()
		bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<AWackyCharacter*, uint32> HeadShots;

	UPROPERTY()
	TMap<AWackyCharacter*, uint32> BodyShots;

};

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	AWackyCharacter* Character;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WACKYWARS_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class AWackyCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& Package, const FColor Color);

	/*
	* Hitscan
	*/
	FServerSideRewindResult ServerSideRewind(
		class AWackyCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize& HitLocation, float HitTime);
	/*
	* Projectile
	*/

	FServerSideRewindResult ProjectileServerSideRewind(
		class AWackyCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	/*
	* Shotgun
	*/
	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(
		const TArray<AWackyCharacter*>& HitCharacters, 
		const FVector_NetQuantize& TraceStart, 
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime
	);

	UFUNCTION(Server, Reliable)
		void ServerScoreRequest(
			AWackyCharacter* HitCharacter,
			const FVector_NetQuantize& TraceStart,
			const FVector_NetQuantize& HitLocation,
			float HitTime
		);

	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(
		class AWackyCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);
protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	void CacheBoxPositions(AWackyCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(AWackyCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(AWackyCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(AWackyCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	void SaveFramePackage();
	FFramePackage GetFrameToCheck(AWackyCharacter* HitCharacter, float HitTime);

	/*
	* Hitscan
	*/

	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		AWackyCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation);

	/*
	* Projectile
	*/

	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		class AWackyCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	/*
	* Shotgun
	*/
	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<AWackyCharacter*>& HitCharacters, 
		const FVector_NetQuantize& TraceStart, 
		const TArray<FVector_NetQuantize>& HitLocations, 
		float HitTime);

	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations);
private:

	UPROPERTY()
	AWackyCharacter* Character;

	UPROPERTY()
	class AWackyPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;

public:	

		
};

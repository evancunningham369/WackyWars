// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "WackyWars/Character/WackyCharacter.h"
#include "WackyWars/WackyComponents/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyInex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyInex, bFromSweep, SweepResult);

	AWackyCharacter* WackyCharacter = Cast<AWackyCharacter>(OtherActor);
	if (WackyCharacter)
	{
		UCombatComponent* Combat = WackyCharacter->GetCombat();
		if (Combat)
		{
			Combat->PickupAmmo(WeaponType, AmmoAmount);
		}
	}
	Destroy();
}
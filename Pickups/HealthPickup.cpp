// Fill out your copyright notice in the Description page of Project Settings.
#include "HealthPickup.h"
#include "WackyWars/Character/WackyCharacter.h"
#include "WackyWars/WackyComponents/BuffComponent.h"

AHealthPickup::AHealthPickup()
{
	bReplicates = true;
}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyInex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyInex, bFromSweep, SweepResult);

	AWackyCharacter* WackyCharacter = Cast<AWackyCharacter>(OtherActor);
	if (WackyCharacter)
	{
		UBuffComponent* Buff = WackyCharacter->GetBuff();
		if (Buff)
		{
			Buff->Heal(HealAmount, HealingTime);
		}
	}
	Destroy();
}
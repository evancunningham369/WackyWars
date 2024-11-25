#include "SpeedPickup.h"
#include "WackyWars/Character/WackyCharacter.h"
#include "WackyWars/WackyComponents/BuffComponent.h"

void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyInex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyInex, bFromSweep, SweepResult);

	AWackyCharacter* WackyCharacter = Cast<AWackyCharacter>(OtherActor);
	if (WackyCharacter)
	{
		UBuffComponent* Buff = WackyCharacter->GetBuff();
		if (Buff)
		{
			Buff->BuffSpeed(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
		}
	}
	Destroy();
}
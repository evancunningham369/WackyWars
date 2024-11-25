#pragma once
#include "CoreMinimal.h"
#include "Pickup.h"
#include "SpeedPickup.generated.h"

/**
 * 
 */
UCLASS()
class WACKYWARS_API ASpeedPickup : public APickup
{
	GENERATED_BODY()
protected:
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
	float BaseSpeedBuff = 1600.f;
	
	UPROPERTY(EditAnywhere)
	float CrouchSpeedBuff = 850.f;

	UPROPERTY(EditAnywhere)
	float SpeedBuffTime = 30.f;
};

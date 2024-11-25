// Fill out your copyright notice in the Description page of Project Settings.


#include "WackyAnimInstance.h"
#include "WackyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "WackyWars/Weapon/Weapon.h"
#include "WackyWars/WackyTypes/CombatState.h"

void UWackyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	WackyCharacter = Cast<AWackyCharacter>(TryGetPawnOwner());
}

void UWackyAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (WackyCharacter == nullptr) {
		WackyCharacter = Cast<AWackyCharacter>(TryGetPawnOwner());
	}

	if (WackyCharacter == nullptr) return;

	FVector Velocity = WackyCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = WackyCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = WackyCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true: false;
	bWeaponEquipped = WackyCharacter->isWeaponEquipped();
	EquippedWeapon = WackyCharacter->GetEquippedWeapon();
	bIsCrouched = WackyCharacter->bIsCrouched;
	bAiming = WackyCharacter->IsAiming();
	TurningInPlace = WackyCharacter->GetTurningInPlace();
	bRotateRootBone = WackyCharacter->ShouldRotateRootBone();
	bElimmed = WackyCharacter->isElimmed();
	bHoldingTheFlag = WackyCharacter->IsHoldingTheFlag();

	// Offset Yaw from Strafing
	FRotator AimRotation = WackyCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(WackyCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = WackyCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = WackyCharacter->GetAO_Yaw();
	AO_Pitch = WackyCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && WackyCharacter->GetMesh()) 
	{

		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		WackyCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (WackyCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - WackyCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}
	}

	bUseFABRIK = WackyCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
	bool bFABRIKOverride =
		WackyCharacter->IsLocallyControlled() &&
		WackyCharacter->GetCombatState() != ECombatState::ECS_ThrowingGrenade &&
		WackyCharacter->bFinishedSwapping;

	if (bFABRIKOverride)
	{
		bUseFABRIK = !WackyCharacter->isLocallyReloading();
	}
	bUseAimOffsets = WackyCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !WackyCharacter->GetDisableGameplay();
	bTransformRightHand = WackyCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !WackyCharacter->GetDisableGameplay();;

}

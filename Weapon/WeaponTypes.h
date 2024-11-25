#pragma once

#define TRACE_LENGTH 80000.f

#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayNames = "Assault Rifle"),
	EWT_RocketLauncher UMETA(DisplayNames = "Rocket Launcher"),
	EWT_Pistol UMETA(DisplayNames = "Pistol"),
	EWT_SubmachineGun UMETA(DisplayNames = "Submachine Gun"),
	EWT_Shotgun UMETA(DisplayNames = "Shotgun"),
	EWT_SniperRifle UMETA(DisplayNames = "Sniper Rifle"),
	EWT_GrenadeLauncher UMETA(DisplayNames = "Grenade Launcher"),
	EWT_Flag UMETA(DisplayNames = "Flag"),

	EWT_MAX UMETA(DisplayNames = "Default Max")
};
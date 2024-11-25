// Fill out your copyright notice in the Description page of Project Settings.


#include "WackyPlayerController.h"
#include "WackyWars/HUD/WackyHUD.h"
#include "WackyWars/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "WackyWars/Character/WackyCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Net/UnrealNetwork.h"
#include "WackyWars/GameMode/WackyGameMode.h"
#include "WackyWars/HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"
#include "WackyWars/WackyComponents/CombatComponent.h"
#include "WackyWars/GameState/WackyGameState.h"
#include "WackyWars/PlayerState/WackyPlayerState.h"
#include "Components/Image.h"
#include "WackyWars/HUD/ReturnToMainMenu.h"
#include "WackyWars/WackyTypes/Announcement.h"

void AWackyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ServerCheckMatchState();

	WackyHUD = Cast<AWackyHUD>(GetHUD());
	ServerCheckMatchState();
}

void AWackyPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
	CheckPing(DeltaTime);
}

void AWackyPlayerController::BrodcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void AWackyPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		WackyHUD = WackyHUD == nullptr ? Cast<AWackyHUD>(GetHUD()) : WackyHUD;
		if (WackyHUD)
		{
			if (Attacker == Self && Victim != Self)
			{
				WackyHUD->AddElimAnnoucement("You", Victim->GetPlayerName());
				return;
			}
			if (Victim == Self && Attacker != Self)
			{
				WackyHUD->AddElimAnnoucement(Attacker->GetPlayerName(), "you");
				return;
			}
			if (Attacker == Victim && Attacker == Self)
			{
				WackyHUD->AddElimAnnoucement("You", "yourself");
				return;
			}
			if (Attacker == Victim && Attacker != Self)
			{
				WackyHUD->AddElimAnnoucement(Attacker->GetPlayerName(), "themselves");
				return;
			}
			WackyHUD->AddElimAnnoucement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}

void AWackyPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;

	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState)
		{
			if (PlayerState->GetPing() * 4 > HighPingThreshold)
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	bool bHighPingAnimationPlaying = WackyHUD &&
		WackyHUD->CharacterOverlay &&
		WackyHUD->CharacterOverlay->HighPingAnimation &&
		WackyHUD->CharacterOverlay->IsAnimationPlaying(WackyHUD->CharacterOverlay->HighPingAnimation);

	if (bHighPingAnimationPlaying)
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void AWackyPlayerController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuWidget == nullptr) return;
	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);
	}
	if (ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

void AWackyPlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}

// Is the ping too high?
void AWackyPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void AWackyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWackyPlayerController, MatchState);
	DOREPLIFETIME(AWackyPlayerController, bShowTeamScores);
}

void AWackyPlayerController::HideTeamScores()
{
	WackyHUD = WackyHUD == nullptr ? Cast<AWackyHUD>(GetHUD()) : WackyHUD;
	bool bHUDValid = WackyHUD &&
		WackyHUD->CharacterOverlay &&
		WackyHUD->CharacterOverlay->RedTeamScore &&
		WackyHUD->CharacterOverlay->BlueTeamScore &&
		WackyHUD->CharacterOverlay->ScoreSpacerText;

	if (bHUDValid)
	{
		WackyHUD->CharacterOverlay->RedTeamScore->SetText(FText());
		WackyHUD->CharacterOverlay->BlueTeamScore->SetText(FText());
		WackyHUD->CharacterOverlay->ScoreSpacerText->SetText(FText());
	}
}

void AWackyPlayerController::InitTeamScores()
{
	WackyHUD = WackyHUD == nullptr ? Cast<AWackyHUD>(GetHUD()) : WackyHUD;
	bool bHUDValid = WackyHUD &&
		WackyHUD->CharacterOverlay &&
		WackyHUD->CharacterOverlay->RedTeamScore &&
		WackyHUD->CharacterOverlay->BlueTeamScore &&
		WackyHUD->CharacterOverlay->ScoreSpacerText;

	if (bHUDValid)
	{
		FString Zero("0");
		FString Spacer("|");
		WackyHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(Zero));
		WackyHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(Zero));
		WackyHUD->CharacterOverlay->ScoreSpacerText->SetText(FText::FromString(Spacer));
	}
}

void AWackyPlayerController::SetHUDRedTeamScore(int32 RedScore)
{
	WackyHUD = WackyHUD == nullptr ? Cast<AWackyHUD>(GetHUD()) : WackyHUD;
	bool bHUDValid = WackyHUD &&
		WackyHUD->CharacterOverlay &&
		WackyHUD->CharacterOverlay->RedTeamScore;

	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), RedScore);
		WackyHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void AWackyPlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	WackyHUD = WackyHUD == nullptr ? Cast<AWackyHUD>(GetHUD()) : WackyHUD;
	bool bHUDValid = WackyHUD &&
		WackyHUD->CharacterOverlay &&
		WackyHUD->CharacterOverlay->BlueTeamScore;

	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);
		WackyHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void AWackyPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

float AWackyPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AWackyPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AWackyPlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AWackyPlayerController::HighPingWarning()
{
	WackyHUD = WackyHUD == nullptr ? Cast<AWackyHUD>(GetHUD()) : WackyHUD;
	bool bHUDValid = WackyHUD &&
		WackyHUD->CharacterOverlay &&
		WackyHUD->CharacterOverlay->HighPingImage &&
		WackyHUD->CharacterOverlay->HighPingAnimation;

	if (bHUDValid)
	{
		WackyHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		WackyHUD->CharacterOverlay->PlayAnimation(WackyHUD->CharacterOverlay->HighPingAnimation,
			0.f,
			5);
	}
}

void AWackyPlayerController::StopHighPingWarning()
{
	WackyHUD = WackyHUD == nullptr ? Cast<AWackyHUD>(GetHUD()) : WackyHUD;
	bool bHUDValid = WackyHUD &&
		WackyHUD->CharacterOverlay &&
		WackyHUD->CharacterOverlay->HighPingImage &&
		WackyHUD->CharacterOverlay->HighPingAnimation;

	if (bHUDValid)
	{
		WackyHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (WackyHUD->CharacterOverlay->IsAnimationPlaying(WackyHUD->CharacterOverlay->HighPingAnimation))
		{
			WackyHUD->CharacterOverlay->StopAnimation(WackyHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

void AWackyPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AWackyPlayerController::ServerCheckMatchState_Implementation()
{
	AWackyGameMode* GameMode = Cast<AWackyGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}

	if (WackyHUD && MatchState == MatchState::WaitingToStart)
	{
		WackyHUD->AddAnnouncement();
	}
}

void AWackyPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if (WackyHUD && MatchState == MatchState::WaitingToStart)
	{
		WackyHUD->AddAnnouncement();
	}
}

void AWackyPlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	if(HasAuthority()) bShowTeamScores = bTeamsMatch;
	WackyHUD = WackyHUD == nullptr ? Cast<AWackyHUD>(GetHUD()) : WackyHUD;
	if (WackyHUD)
	{
		if (WackyHUD->CharacterOverlay == nullptr) WackyHUD->AddCharacterOverlay();
		if (WackyHUD->Announcement)
		{
			WackyHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		if (!HasAuthority()) return;
		if (bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}

void AWackyPlayerController::HandleCooldown()
{
	WackyHUD = WackyHUD == nullptr ? Cast<AWackyHUD>(GetHUD()) : WackyHUD;
	if (WackyHUD)
	{
		WackyHUD->CharacterOverlay->RemoveFromParent();
		bool bHUDValid = WackyHUD->Announcement &&
			WackyHUD->Announcement->AnnouncementText &&
			WackyHUD->Announcement->InfoText;

		if (bHUDValid)
		{
			WackyHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText = Announcement::NewMatchStartsIn;
			WackyHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			AWackyGameState* WackyGameState = Cast<AWackyGameState>(UGameplayStatics::GetGameState(this));
			AWackyPlayerState* WackyPlayerState = GetPlayerState<AWackyPlayerState>();
			if (WackyGameState && WackyPlayerState)
			{
				TArray<AWackyPlayerState*> TopPlayers = WackyGameState->TopScoringPlayers;
				FString InfoTextString = bShowTeamScores ? GetTeamsInfoText(WackyGameState) : GetInfoText(TopPlayers);

				WackyHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	AWackyCharacter* WackyCharacter = Cast<AWackyCharacter>(GetPawn());
	if (WackyCharacter && WackyCharacter->GetCombat())
	{
		WackyCharacter->bDisableGameplay = true;
		WackyCharacter->GetCombat()->FireButtonPressed(false);
	}
}

FString AWackyPlayerController::GetInfoText(const TArray<class AWackyPlayerState*>& Players)
{
	AWackyPlayerState* WackyPlayerState = GetPlayerState<AWackyPlayerState>();
	if (WackyPlayerState == nullptr) return FString();
	FString InfoTextString;

	if (Players.Num() == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (Players.Num() == 1 && Players[0] == WackyPlayerState)
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}
	else if (Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *Players[0]->GetPlayerName());
	}
	else if (Players.Num() > 1)
	{
		InfoTextString = Announcement::PlayersTiedForTheWin;
		InfoTextString.Append(FString("\n"));
		for (auto TiedPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}
	return InfoTextString;
}

FString AWackyPlayerController::GetTeamsInfoText(AWackyGameState* WackyGameState)
{
	if (WackyGameState == nullptr) return FString();
	FString InfoTextString;

	const int32 RedTeamScore = WackyGameState->RedTeamScore;
	const int32 BlueTeamScore = WackyGameState->BlueTeamScore;

	if (RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (RedTeamScore == BlueTeamScore)
	{
		InfoTextString = FString::Printf(TEXT("%s\n"), *Announcement::TeamsTiedForTheWin);
		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(Announcement::BlueTeam);
		InfoTextString.Append(TEXT("\n"));
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s : %d\n"), *Announcement::RedTeam, RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s : %d\n"), *Announcement::BlueTeam, BlueTeamScore));
	}
	else if (BlueTeamScore > RedTeamScore)
	{
		InfoTextString = Announcement::BlueTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s : %d\n"), *Announcement::BlueTeam, BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s : %d\n"), *Announcement::RedTeam, RedTeamScore));
	}

	return InfoTextString;
}

void AWackyPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (HasAuthority())
	{
		WackyGameMode = WackyGameMode == nullptr ? Cast<AWackyGameMode>(UGameplayStatics::GetGameMode(this)) : WackyGameMode;
		if (WackyGameMode)
		{
			SecondsLeft = FMath::CeilToInt(WackyGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	CountdownInt = SecondsLeft;
}

void AWackyPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (WackyHUD && WackyHUD->CharacterOverlay)
		{
			CharacterOverlay = WackyHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);

				AWackyCharacter* WackyCharacter = Cast<AWackyCharacter>(GetPawn());
				if (WackyCharacter && WackyCharacter->GetCombat())
				{
					if (bInitializeGrenades) SetHUDGrenades(WackyCharacter->GetCombat()->GetGrenades());
				}
			}
		}
	}
}

void AWackyPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AWackyPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = (0.5f * RoundTripTime);
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void AWackyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AWackyCharacter* WackyCharacter = Cast<AWackyCharacter>(InPawn);
	if (WackyCharacter)
	{
		SetHUDHealth(WackyCharacter->GetHealth(), WackyCharacter->GetMaxHealth());
	}
}

void AWackyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(QuitAction, ETriggerEvent::Started, this, &ThisClass::ShowReturnToMainMenu);
	}
}

void AWackyPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	WackyHUD = WackyHUD == nullptr ? Cast<AWackyHUD>(GetHUD()) : WackyHUD;
	bool bHUDValid = WackyHUD &&
		WackyHUD->CharacterOverlay &&
		WackyHUD->CharacterOverlay->HealthBar &&
		WackyHUD->CharacterOverlay->HealthText;

	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		WackyHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		WackyHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AWackyPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	WackyHUD = WackyHUD == nullptr ? Cast<AWackyHUD>(GetHUD()) : WackyHUD;
	bool bHUDValid = WackyHUD &&
		WackyHUD->CharacterOverlay &&
		WackyHUD->CharacterOverlay->ShieldBar &&
		WackyHUD->CharacterOverlay->ShieldText;

	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;
		WackyHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		WackyHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void AWackyPlayerController::SetHUDScore(float Score)
{
	WackyHUD = WackyHUD == nullptr ? Cast<AWackyHUD>(GetHUD()) : WackyHUD;
	bool bHUDValid = WackyHUD &&
		WackyHUD->CharacterOverlay &&
		WackyHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		WackyHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void AWackyPlayerController::SetHUDDefeats(int32 Defeats)
{
	WackyHUD = WackyHUD == nullptr ? Cast<AWackyHUD>(GetHUD()) : WackyHUD;
	bool bHUDValid = WackyHUD &&
		WackyHUD->CharacterOverlay &&
		WackyHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		WackyHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void AWackyPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	WackyHUD = WackyHUD == nullptr ? Cast<AWackyHUD>(GetHUD()) : WackyHUD;
	bool bHUDValid = WackyHUD &&
		WackyHUD->CharacterOverlay &&
		WackyHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		WackyHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void AWackyPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	WackyHUD = WackyHUD == nullptr ? Cast<AWackyHUD>(GetHUD()) : WackyHUD;
	bool bHUDValid = WackyHUD &&
		WackyHUD->CharacterOverlay &&
		WackyHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		FString CarriedAmmoText = FString::Printf(TEXT("%d"), Ammo);
		WackyHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void AWackyPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	WackyHUD = WackyHUD == nullptr ? Cast<AWackyHUD>(GetHUD()) : WackyHUD;
	bool bHUDValid = WackyHUD &&
		WackyHUD->CharacterOverlay &&
		WackyHUD->CharacterOverlay->MatchCountdownText;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			WackyHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		WackyHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AWackyPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	WackyHUD = WackyHUD == nullptr ? Cast<AWackyHUD>(GetHUD()) : WackyHUD;
	bool bHUDValid = WackyHUD &&
		WackyHUD->Announcement &&
		WackyHUD->Announcement->WarmupTime;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			WackyHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		WackyHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void AWackyPlayerController::SetHUDGrenades(int32 Grenades)
{
	WackyHUD = WackyHUD == nullptr ? Cast<AWackyHUD>(GetHUD()) : WackyHUD;
	bool bHUDValid = WackyHUD &&
		WackyHUD->CharacterOverlay &&
		WackyHUD->CharacterOverlay->GrenadesText;
	if (bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		WackyHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
}



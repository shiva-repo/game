#include "RocketDodgerGameMode.h"
#include "MeteorActor.h"
#include "PowerupActor.h"
#include "RocketPawn.h"
#include "RocketDodgerHUD.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

ARocketDodgerGameMode::ARocketDodgerGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	Score = 0;
	Lives = 3;
	Level = 1;
	Difficulty = TEXT("medium");
	PilotName = TEXT("Pilot");

	bGameStarted = false;
	bGamePaused = false;
	bGameOver = false;
	bIsBossBattle = false;

	SpawnCooldown = 1.2f;
	SpawnTimer = 0.f;
	PowerupTimer = 0.f;

	MeteorClass = AMeteorActor::StaticClass();
	PowerupClass = APowerupActor::StaticClass();
	DefaultPawnClass = ARocketPawn::StaticClass();
	HUDClass = ARocketDodgerHUD::StaticClass();
}

void ARocketDodgerGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// --- TRANSFORM DEFAULT DAYTIME WORLD INTO DEEP DARK OUTER SPACE ---
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		FString ClassName = It->GetClass()->GetName();
		FString ActorName = It->GetName();

		// We want to keep DirectionalLight so our spaceship is beautifully lit, but destroy SkyLight to eliminate the red real-time capture warning!
		if (ClassName.Contains(TEXT("DirectionalLight")))
		{
			continue;
		}

		// Hide and destroy everything else related to the default daytime environment (Clouds, SkyLight, Fog, Atmosphere, Landscape, Floor grids, Foliage, etc.)
		if (ClassName.Contains(TEXT("VolumetricCloud")) || ClassName.Contains(TEXT("SkyAtmosphere")) || 
			ClassName.Contains(TEXT("ExponentialHeightFog")) || ClassName.Contains(TEXT("Landscape")) || 
			ClassName.Contains(TEXT("StaticMeshActor")) || ClassName.Contains(TEXT("InstancedFoliage")) || 
			ClassName.Contains(TEXT("WorldPartitionMiniMap")) || ClassName.Contains(TEXT("SkyLight")) || 
			ActorName.Contains(TEXT("Floor")) || ActorName.Contains(TEXT("Grid")) || 
			ActorName.Contains(TEXT("Plane")) || ActorName.Contains(TEXT("Cube")) || 
			ActorName.Contains(TEXT("Cloud")) || ActorName.Contains(TEXT("Fog")) || 
			ActorName.Contains(TEXT("Sky")) || ActorName.Contains(TEXT("Landscape")))
		{
			It->SetActorHiddenInGame(true);
			It->SetActorEnableCollision(false);
			It->Destroy();
		}
	}

	// Game starts on menu; widgets handle calling StartNewGame
	PlaySynthSound(TEXT("menu_music"));
}

void ARocketDodgerGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	if (!World) return;

	// --- SPAWN DYNAMIC 3D FLYING STARFIELD (Always active in menu and gameplay!) ---
	static float StarTimer = 0.0f;
	StarTimer += DeltaTime;
	if (StarTimer >= 0.08f)
	{
		StarTimer = 0.0f;
		float StarX = FMath::RandRange(-1800.f, 1800.f);
		float StarZ = FMath::RandRange(-1200.f, 1200.f);
		FVector StarLoc(StarX, 4000.f, StarZ);
		AMeteorActor* Star = World->SpawnActor<AMeteorActor>(MeteorClass, StarLoc, FRotator::ZeroRotator);
		if (Star)
		{
			Star->ObstacleType = TEXT("star");
			Star->BaseSpeed = FMath::RandRange(800.f, 1400.f);
		}
	}

	if (!bGameStarted || bGameOver || bGamePaused) return;

	// Update power-up spawn timer
	PowerupTimer += DeltaTime;
	if (PowerupTimer >= 8.0f)
	{
		PowerupTimer = 0.f;
		SpawnPowerupItem();
	}

	// In Boss battles, regular meteors pause
	if (bIsBossBattle) return;

	// Spawning meteors
	SpawnTimer += DeltaTime;
	float AdjustedCooldown = SpawnCooldown * GetSpawnIntervalMultiplier();
	if (SpawnTimer >= AdjustedCooldown)
	{
		SpawnTimer = 0.f;
		SpawnMeteor();
	}
}

void ARocketDodgerGameMode::StartNewGame(FString NewPilotName)
{
	PilotName = NewPilotName;
	Score = 0;
	Lives = 3;
	Level = 1;
	bGameStarted = true;
	bGameOver = false;
	bGamePaused = false;
	bIsBossBattle = false;
	SpawnTimer = 0.f;
	PowerupTimer = 0.f;

	// Spawn Player Pawn
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		ARocketPawn* ExistingPawn = Cast<ARocketPawn>(PC->GetPawn());
		if (!ExistingPawn)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			FVector SpawnLoc(0.f, 0.f, 0.f); // Player sits at Y=0 (depth)
			ARocketPawn* Pawn = PC->GetWorld()->SpawnActor<ARocketPawn>(ARocketPawn::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
			if (Pawn)
			{
				PC->Possess(Pawn);
				Pawn->BombCount = 0;
			}
		}
		else
		{
			ExistingPawn->BombCount = 0;
			ExistingPawn->SetActorLocation(FVector(0.f, 0.f, 0.f));
		}
	}

	OnScoreUpdated(Score, Level);
	OnLivesUpdated(Lives);
	UpdateBombHUD();
	PlaySynthSound(TEXT("game_music"));
}

void ARocketDodgerGameMode::AwardScore(int32 Amount)
{
	if (bGameOver) return;

	Score += Amount;
	
	// Check Level Up (Every 1000 points triggers a Level Up!)
	int32 TargetLevel = (Score / 1000) + 1;
	if (TargetLevel > Level)
	{
		Level = TargetLevel;
		PlaySynthSound(TEXT("level_up"));

		// Check if we should trigger a Boss Battle! (Every 5 levels: level 5, 10, 15...)
		if (Level % 5 == 0)
		{
			StartBossBattle();
		}
	}

	OnScoreUpdated(Score, Level);
}

void ARocketDodgerGameMode::ReduceLives()
{
	if (bGameOver) return;

	Lives--;
	PlaySynthSound(TEXT("player_hit"));
	OnLivesUpdated(Lives);

	if (Lives <= 0)
	{
		bGameOver = true;
		PlaySynthSound(TEXT("player_explosion"));
		
		// Hide ship, launch ejection sequence
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (PC)
		{
			ARocketPawn* Rocket = Cast<ARocketPawn>(PC->GetPawn());
			if (Rocket)
			{
				Rocket->SetActorEnableCollision(false);
				Rocket->RocketMesh->SetVisibility(false);
				Rocket->LeftWingMesh->SetVisibility(false);
				Rocket->RightWingMesh->SetVisibility(false);
				// Triggers flying astronaut spin/shrink in Blueprints!
			}
		}

		// Wait 2 seconds (ejection delay) before displaying Game Over menu
		FTimerHandle GameOverDelayHandle;
		GetWorldTimerManager().SetTimer(GameOverDelayHandle, [this]() {
			OnGameOver(Score);
		}, 2.0f, false);
	}
}

void ARocketDodgerGameMode::SpawnMeteor()
{
	UWorld* World = GetWorld();
	if (!World || !MeteorClass) return;

	// Spawns deep in the screen (Y=3000), moving towards camera
	float RandX = FMath::RandRange(-600.f, 600.f);
	float RandZ = FMath::RandRange(-400.f, 400.f); // spawn at random heights
	FVector SpawnLoc(RandX, 3000.f, RandZ);

	AMeteorActor* Meteor = World->SpawnActor<AMeteorActor>(MeteorClass, SpawnLoc, FRotator::ZeroRotator);
	if (Meteor)
	{
		// 70% normal, 15% splitter, 15% space junk
		float TypeRoll = FMath::FRand();
		if (TypeRoll < 0.15f)
		{
			Meteor->ObstacleType = TEXT("splitter");
			Meteor->MaxHealth = 1;
			Meteor->BaseSpeed = 350.f + (Level * 20.f);
		}
		else if (TypeRoll < 0.30f)
		{
			Meteor->ObstacleType = TEXT("spacejunk");
			Meteor->MaxHealth = 2; // satellite requires 2 hits
			Meteor->BaseSpeed = 300.f + (Level * 15.f);
		}
		else
		{
			Meteor->ObstacleType = TEXT("normal");
			Meteor->MaxHealth = 1;
			Meteor->BaseSpeed = 400.f + (Level * 25.f);
		}

		// Increase speed depending on difficulty setting
		if (Difficulty == TEXT("easy")) Meteor->BaseSpeed *= 0.7f;
		else if (Difficulty == TEXT("hard")) Meteor->BaseSpeed *= 1.3f;
		else if (Difficulty == TEXT("impossible")) Meteor->BaseSpeed *= 1.7f;
		else if (Difficulty == TEXT("asian")) Meteor->BaseSpeed *= 2.5f; // Super insane speed!
	}
}

void ARocketDodgerGameMode::SpawnPowerupItem()
{
	UWorld* World = GetWorld();
	if (!World || !PowerupClass) return;

	float RandX = FMath::RandRange(-600.f, 600.f);
	float RandZ = FMath::RandRange(-400.f, 400.f);
	FVector SpawnLoc(RandX, 3000.f, RandZ);

	APowerupActor* Powerup = World->SpawnActor<APowerupActor>(PowerupClass, SpawnLoc, FRotator::ZeroRotator);
	if (Powerup)
	{
		float TypeRoll = FMath::FRand();
		if (TypeRoll < 0.4f)
		{
			Powerup->PowerupType = TEXT("shield");
		}
		else if (TypeRoll < 0.7f)
		{
			Powerup->PowerupType = TEXT("triple");
		}
		else
		{
			Powerup->PowerupType = TEXT("bomb");
		}
	}
}

void ARocketDodgerGameMode::StartBossBattle()
{
	bIsBossBattle = true;
	PlaySynthSound(TEXT("siren_alarm"));

	// Clear out current hazards
	for (TActorIterator<AMeteorActor> It(GetWorld()); It; ++It)
	{
		if (It->ObstacleType != TEXT("boss"))
		{
			It->Destroy();
		}
	}

	// Spawn Giant Alien Boss
	UWorld* World = GetWorld();
	if (World && MeteorClass)
	{
		FVector SpawnLoc(0.f, 3200.f, 150.f);
		AMeteorActor* Boss = World->SpawnActor<AMeteorActor>(MeteorClass, SpawnLoc, FRotator::ZeroRotator);
		if (Boss)
		{
			Boss->ObstacleType = TEXT("boss");
			Boss->MaxHealth = 10 + (Level / 5 - 1) * 5;
			Boss->BaseSpeed = 300.f;
			Boss->SetActorScale3D(FVector(3.5f)); // Boss is huge!
		}
	}
}

void ARocketDodgerGameMode::OnBossDefeated()
{
	bIsBossBattle = false;
	AwardScore(500); // 500 bonus points!
	PlaySynthSound(TEXT("boss_defeat"));
}

void ARocketDodgerGameMode::TriggerBombShockwave(FVector BlastLocation)
{
	// Trigger Blueprint visual expansion wave
	OnSpawnBombShockwave(BlastLocation);

	// Destroy all enemies and hazards within 1800 units radius
	float BlastRadius = 1800.0f;

	for (TActorIterator<AMeteorActor> It(GetWorld()); It; ++It)
	{
		float Dist = FVector::Dist(BlastLocation, It->GetActorLocation());
		if (Dist <= BlastRadius)
		{
			if (It->ObstacleType == TEXT("boss"))
			{
				It->ApplyMeteorDamage(3); // Bomb deals high damage to boss
			}
			else
			{
				It->ApplyMeteorDamage(10); // Vaporize regular meteors
			}
		}
	}
}

void ARocketDodgerGameMode::UpdateBombHUD()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		ARocketPawn* Rocket = Cast<ARocketPawn>(PC->GetPawn());
		if (Rocket)
		{
			OnBombsUpdated(Rocket->BombCount);
		}
	}
}

float ARocketDodgerGameMode::GetSpawnIntervalMultiplier() const
{
	if (Difficulty == TEXT("easy")) return 1.5f; // spawns slower
	if (Difficulty == TEXT("hard")) return 0.7f; // spawns faster
	if (Difficulty == TEXT("impossible")) return 0.5f;
	if (Difficulty == TEXT("asian")) return 0.3f; // spawns at crazy speed!
	return 1.0f; // medium
}

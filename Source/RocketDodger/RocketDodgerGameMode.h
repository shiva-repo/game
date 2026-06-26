#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RocketDodgerGameMode.generated.h"

class AMeteorActor;
class APowerupActor;

UCLASS()
class ROCKETDODGER_API ARocketDodgerGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARocketDodgerGameMode();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// Spawn Classes (assigned in Blueprint editor)
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<AMeteorActor> MeteorClass;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<APowerupActor> PowerupClass;

	// Spawning timers & settings
	float SpawnCooldown;
	float SpawnTimer;
	float PowerupTimer;

	// Game metrics
	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	int32 Score;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	int32 Lives;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	int32 Level;

	UPROPERTY(BlueprintReadWrite, Category = "Gameplay")
	FString Difficulty; // "easy", "medium", "hard", "impossible", "asian"

	UPROPERTY(BlueprintReadWrite, Category = "Gameplay")
	FString PilotName;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	bool bGameStarted;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	bool bGamePaused;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	bool bGameOver;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	bool bIsBossBattle;

	// Actions called by player or hazards
	void StartNewGame(FString NewPilotName);
	void AwardScore(int32 Amount);
	void ReduceLives();
	void OnBossDefeated();
	void TriggerBombShockwave(FVector BlastLocation);
	void UpdateBombHUD();

	// Blueprint hooks for UI, effects, and synth audio
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnScoreUpdated(int32 NewScore, int32 NewLevel);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnLivesUpdated(int32 NewLives);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnBombsUpdated(int32 NewBombs);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnGameOver(int32 FinalScore);

	UFUNCTION(BlueprintImplementableEvent, Category = "Gameplay")
	void PlaySynthSound(const FString& SoundType);

	UFUNCTION(BlueprintImplementableEvent, Category = "VFX")
	void OnSpawnBombShockwave(FVector BlastLocation);

private:
	void SpawnMeteor();
	void SpawnPowerupItem();
	void StartBossBattle();
	float GetSpawnIntervalMultiplier() const;
};

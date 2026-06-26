#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RocketDodgerHUD.generated.h"

USTRUCT(BlueprintType)
struct FPilotRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Score;

	FPilotRecord() : Name(TEXT("Pilot")), Score(0) {}
	FPilotRecord(FString InName, int32 InScore) : Name(InName), Score(InScore) {}
};

UCLASS()
class ROCKETDODGER_API ARocketDodgerHUD : public AHUD
{
	GENERATED_BODY()

public:
	ARocketDodgerHUD();

	virtual void DrawHUD() override;

protected:
	virtual void BeginPlay() override;

private:
	// Selected options on the main menu
	FString SelectedDifficulty;
	FString SelectedSkin;
	
	// Customizations
	FString CustomSkinTone;
	FString CustomHairColor;
	FString CustomVisorColor;

	// Mouse State
	bool bWasPressedLastFrame;

	// Leaderboard Cache
	TArray<FPilotRecord> Leaderboard;

	// Helper functions to draw UI elements
	bool DrawButton(const FString& Text, float X, float Y, float Width, float Height, FLinearColor BaseColor, FLinearColor HoverColor, FLinearColor TextColor);
	void DrawWindowOverlay(const FString& Title, const FString& Subtitle);
	
	// Leaderboard Storage
	void LoadLeaderboard();
	void SaveScoreToLeaderboard(FString Name, int32 Score);
};

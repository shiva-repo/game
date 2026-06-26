#include "RocketDodgerHUD.h"
#include "RocketDodgerGameMode.h"
#include "RocketPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Canvas.h"
#include "Misc/FileHelper.h"

ARocketDodgerHUD::ARocketDodgerHUD()
{
	SelectedDifficulty = TEXT("medium");
	SelectedSkin = TEXT("classic");
	CustomSkinTone = TEXT("peach");
	CustomHairColor = TEXT("blonde");
	CustomVisorColor = TEXT("gold");
	bWasPressedLastFrame = false;
}

void ARocketDodgerHUD::BeginPlay()
{
	Super::BeginPlay();
	LoadLeaderboard();

	APlayerController* PC = GetOwningPlayerController();
	if (PC)
	{
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
	}
}

void ARocketDodgerHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas) return;

	ARocketDodgerGameMode* GameMode = Cast<ARocketDodgerGameMode>(UGameplayStatics::GetGameMode(this));
	if (!GameMode) return;

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	bool bIsPressed = PC->IsInputKeyDown(EKeys::LeftMouseButton);
	bool bClick = bIsPressed && !bWasPressedLastFrame;

	// Draw HUD elements based on game states
	if (!GameMode->bGameStarted)
	{
		// ==========================================
		//           🎮 START SCREEN UI
		// ==========================================
		DrawRect(FLinearColor(0.03f, 0.04f, 0.08f, 1.0f), 0.f, 0.f, Canvas->SizeX, Canvas->SizeY);

		// Dynamic Stars background simulation (just drawing some static/moving points in HUD for retro feel)
		for (int32 i = 0; i < 30; ++i)
		{
			float starX = FMath::RandRange(0.f, (float)Canvas->SizeX);
			float starY = FMath::RandRange(0.f, (float)Canvas->SizeY);
			DrawRect(FLinearColor::White, starX, starY, 2.0f, 2.0f);
		}

		// Title
		DrawText(TEXT("ROCKET DODGER C++"), FLinearColor(0.28f, 0.79f, 0.89f), Canvas->SizeX * 0.5f - 180.f, 60.f, nullptr, 2.5f);
		DrawText(TEXT("UNREAL ENGINE 100% C++ PORT"), FLinearColor(1.0f, 0.72f, 0.01f), Canvas->SizeX * 0.5f - 150.f, 120.f, nullptr, 1.0f);

		float MidX = Canvas->SizeX * 0.5f;

		// 1. Pilot Preset Selector
		DrawText(TEXT("PILOT SELECT:"), FLinearColor::White, MidX - 180.f, 180.f, nullptr, 1.2f);
		if (DrawButton(TEXT("ACE"), MidX - 70.f, 175.f, 60.f, 25.f, FLinearColor::Gray, FLinearColor(0.15f, 0.15f, 0.15f, 1.f), FLinearColor::White))
		{
			GameMode->PilotName = TEXT("ACE");
		}
		if (DrawButton(TEXT("VIPER"), MidX, 175.f, 60.f, 25.f, FLinearColor::Gray, FLinearColor(0.15f, 0.15f, 0.15f, 1.f), FLinearColor::White))
		{
			GameMode->PilotName = TEXT("VIPER");
		}
		if (DrawButton(TEXT("MAVERICK"), MidX + 70.f, 175.f, 90.f, 25.f, FLinearColor::Gray, FLinearColor(0.15f, 0.15f, 0.15f, 1.f), FLinearColor::White))
		{
			GameMode->PilotName = TEXT("MAVERICK");
		}

		// Show Current Name
		FString NameLabel = TEXT("Selected Pilot: ") + GameMode->PilotName;
		DrawText(NameLabel, FLinearColor::Green, MidX - 180.f, 215.f, nullptr, 1.1f);

		// 2. Difficulty Presets
		DrawText(TEXT("DIFFICULTY:"), FLinearColor::White, MidX - 180.f, 250.f, nullptr, 1.2f);
		FLinearColor activeDiffColor(1.0f, 0.3f, 0.4f);
		if (DrawButton(TEXT("EASY"), MidX - 70.f, 245.f, 60.f, 25.f, SelectedDifficulty == TEXT("easy") ? activeDiffColor : FLinearColor::Gray, FLinearColor(0.15f, 0.15f, 0.15f, 1.f), FLinearColor::White))
		{
			SelectedDifficulty = TEXT("easy");
			GameMode->Difficulty = TEXT("easy");
		}
		if (DrawButton(TEXT("MEDIUM"), MidX, 245.f, 70.f, 25.f, SelectedDifficulty == TEXT("medium") ? activeDiffColor : FLinearColor::Gray, FLinearColor(0.15f, 0.15f, 0.15f, 1.f), FLinearColor::White))
		{
			SelectedDifficulty = TEXT("medium");
			GameMode->Difficulty = TEXT("medium");
		}
		if (DrawButton(TEXT("HARD"), MidX + 75.f, 245.f, 60.f, 25.f, SelectedDifficulty == TEXT("hard") ? activeDiffColor : FLinearColor::Gray, FLinearColor(0.15f, 0.15f, 0.15f, 1.f), FLinearColor::White))
		{
			SelectedDifficulty = TEXT("hard");
			GameMode->Difficulty = TEXT("hard");
		}
		if (DrawButton(TEXT("ASIAN"), MidX + 140.f, 245.f, 65.f, 25.f, SelectedDifficulty == TEXT("asian") ? activeDiffColor : FLinearColor::Gray, FLinearColor(0.15f, 0.15f, 0.15f, 1.f), FLinearColor::White))
		{
			SelectedDifficulty = TEXT("asian");
			GameMode->Difficulty = TEXT("asian");
		}

		// 3. Customizations (Visor / Hair / Skin)
		DrawText(TEXT("VISOR COLOR:"), FLinearColor::White, MidX - 180.f, 300.f, nullptr, 1.1f);
		if (DrawButton(TEXT("GOLD"), MidX - 60.f, 295.f, 50.f, 25.f, CustomVisorColor == TEXT("gold") ? FLinearColor::Yellow : FLinearColor::Gray, FLinearColor(0.15f, 0.15f, 0.15f, 1.f), FLinearColor::White)) CustomVisorColor = TEXT("gold");
		if (DrawButton(TEXT("DARK"), MidX - 5.f, 295.f, 50.f, 25.f, CustomVisorColor == TEXT("dark") ? FLinearColor::Black : FLinearColor::Gray, FLinearColor(0.15f, 0.15f, 0.15f, 1.f), FLinearColor::White)) CustomVisorColor = TEXT("dark");
		if (DrawButton(TEXT("CYAN"), MidX + 50.f, 295.f, 50.f, 25.f, CustomVisorColor == TEXT("cyan") ? FLinearColor(0.f, 1.f, 1.f, 1.f) : FLinearColor::Gray, FLinearColor(0.15f, 0.15f, 0.15f, 1.f), FLinearColor::White)) CustomVisorColor = TEXT("cyan");

		// 4. Launcher Action
		if (DrawButton(TEXT("🚀 LAUNCH GAME 🚀"), MidX - 120.f, 380.f, 240.f, 50.f, FLinearColor(0.28f, 0.79f, 0.89f), FLinearColor(0.18f, 0.69f, 0.79f), FLinearColor::Black))
		{
			GameMode->StartNewGame(GameMode->PilotName);
		}

		// 5. Leaderboard UI rendering
		DrawText(TEXT("🏆 LEADERBOARD RANKINGS"), FLinearColor::Yellow, MidX - 130.f, 460.f, nullptr, 1.3f);
		float RankY = 495.f;
		for (int32 r = 0; r < FMath::Min(5, Leaderboard.Num()); ++r)
		{
			FString RankStr = FString::Printf(TEXT("%d. %s  ------  %d"), r + 1, *Leaderboard[r].Name, Leaderboard[r].Score);
			DrawText(RankStr, FLinearColor::White, MidX - 100.f, RankY, nullptr, 1.1f);
			RankY += 25.f;
		}
	}
	else if (GameMode->bGameOver)
	{
		// ==========================================
		//           💀 GAME OVER SCREEN UI
		// ==========================================
		DrawRect(FLinearColor(0.08f, 0.02f, 0.03f, 0.85f), 0.f, 0.f, Canvas->SizeX, Canvas->SizeY);

		DrawText(TEXT("GAME OVER"), FLinearColor::Red, Canvas->SizeX * 0.5f - 110.f, Canvas->SizeY * 0.3f, nullptr, 3.0f);
		
		FString ScoreStr = FString::Printf(TEXT("Final Score: %d"), GameMode->Score);
		DrawText(ScoreStr, FLinearColor::White, Canvas->SizeX * 0.5f - 80.f, Canvas->SizeY * 0.42f, nullptr, 1.5f);

		// Save score once when game over initiates
		static bool bScoreSaved = false;
		if (!bScoreSaved)
		{
			SaveScoreToLeaderboard(GameMode->PilotName, GameMode->Score);
			bScoreSaved = true;
		}

		if (DrawButton(TEXT("PLAY AGAIN"), Canvas->SizeX * 0.5f - 90.f, Canvas->SizeY * 0.52f, 180.f, 40.f, FLinearColor(1.0f, 0.72f, 0.01f), FLinearColor(0.9f, 0.62f, 0.01f), FLinearColor::Black))
		{
			bScoreSaved = false;
			GameMode->StartNewGame(GameMode->PilotName);
		}
	}
	else if (GameMode->bGamePaused)
	{
		// ==========================================
		//           ⏸️ GAME PAUSED SCREEN UI
		// ==========================================
		DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.75f), 0.f, 0.f, Canvas->SizeX, Canvas->SizeY);

		DrawText(TEXT("GAME PAUSED"), FLinearColor::Yellow, Canvas->SizeX * 0.5f - 110.f, Canvas->SizeY * 0.35f, nullptr, 2.5f);
		DrawText(TEXT("Take a breath, space pilot!"), FLinearColor::White, Canvas->SizeX * 0.5f - 120.f, Canvas->SizeY * 0.43f, nullptr, 1.2f);

		if (DrawButton(TEXT("RESUME GAME"), Canvas->SizeX * 0.5f - 90.f, Canvas->SizeY * 0.52f, 180.f, 40.f, FLinearColor(0.0f, 0.96f, 0.83f), FLinearColor(0.0f, 0.8f, 0.7f), FLinearColor::Black))
		{
			GameMode->bGamePaused = false;
		}
	}
	else
	{
		// ==========================================
		//           🚀 ACTIVE GAMEPLAY HUD
		// ==========================================
		
		// 1. Top row Left: Score Board
		FString ScoreStr = GameMode->PilotName.ToUpper() + FString::Printf(TEXT(" | LVL %d | SCORE: %d"), GameMode->Level, GameMode->Score);
		DrawRect(FLinearColor(0.05f, 0.06f, 0.12f, 0.85f), 15.f, 15.f, 250.f, 32.f);
		DrawRect(FLinearColor(0.28f, 0.79f, 0.89f, 0.5f), 15.f, 15.f, 250.f, 2.f); // Neon top border
		DrawRect(FLinearColor(0.28f, 0.79f, 0.89f, 0.5f), 15.f, 45.f, 250.f, 2.f); // Neon bottom border
		DrawText(ScoreStr, FLinearColor(0.28f, 0.79f, 0.89f), 25.f, 21.f, nullptr, 1.1f);

		// 2. Top row Right: Lives Board
		FString LivesStr = TEXT("LIVES: ");
		for (int32 l = 0; l < GameMode->Lives; ++l) LivesStr += TEXT("❤️ ");
		float livesW, livesH;
		GetTextSize(LivesStr, livesW, livesH, nullptr, 1.1f);
		float livesX = Canvas->SizeX - livesW - 35.f;
		DrawRect(FLinearColor(0.05f, 0.06f, 0.12f, 0.85f), livesX - 10.f, 15.f, livesW + 20.f, 32.f);
		DrawRect(FLinearColor(1.0f, 0.2f, 0.2f, 0.5f), livesX - 10.f, 15.f, livesW + 20.f, 2.f); // Red neon top border
		DrawRect(FLinearColor(1.0f, 0.2f, 0.2f, 0.5f), livesX - 10.f, 45.f, livesW + 20.f, 2.f); // Red neon bottom border
		DrawText(LivesStr, FLinearColor::Red, livesX, 21.f, nullptr, 1.1f);

		// 3. Below Lives: Bomb HUD (Aligned Stacked Capsule!)
		ARocketPawn* Rocket = Cast<ARocketPawn>(PC->GetPawn());
		int32 bombs = Rocket ? Rocket->BombCount : 0;
		FString BombStr = FString::Printf(TEXT("💣 %d"), bombs);
		float bombW, bombH;
		GetTextSize(BombStr, bombW, bombH, nullptr, 1.1f);
		float bombX = Canvas->SizeX - bombW - 35.f;
		DrawRect(FLinearColor(0.05f, 0.06f, 0.12f, 0.85f), bombX - 10.f, 52.f, bombW + 20.f, 30.f);
		DrawRect(FLinearColor(1.0f, 0.3f, 0.43f, 0.5f), bombX - 10.f, 52.f, bombW + 20.f, 2.f);
		DrawRect(FLinearColor(1.0f, 0.3f, 0.43f, 0.5f), bombX - 10.f, 80.f, bombW + 20.f, 2.f);
		DrawText(BombStr, FLinearColor(1.0f, 0.3f, 0.43f), bombX, 58.f, nullptr, 1.1f);

		// 4. Settings Bar Stack (Aligned Left, underneath Score Board!)
		// Row size: 190px wide, fits perfectly below score board
		float barX = 15.f;
		float barY = 52.f;
		DrawRect(FLinearColor(0.05f, 0.06f, 0.12f, 0.85f), barX, barY, 195.f, 30.f);
		DrawRect(FLinearColor(0.28f, 0.79f, 0.89f, 0.5f), barX, barY, 195.f, 2.f);
		DrawRect(FLinearColor(0.28f, 0.79f, 0.89f, 0.5f), barX, barY + 28.f, 195.f, 2.f);
		
		// Button: PAUSE
		if (DrawButton(TEXT("⏸"), barX + 5.f, barY + 3.f, 35.f, 24.f, FLinearColor::Transparent, FLinearColor(0.2f, 0.2f, 0.3f), FLinearColor::Yellow))
		{
			GameMode->bGamePaused = true;
		}
		// Divider 1
		DrawRect(FLinearColor(0.5f, 0.5f, 0.5f, 0.3f), barX + 45.f, barY + 8.f, 1.f, 14.f);

		// Button: VIEW
		if (DrawButton(TEXT("📷"), barX + 52.f, barY + 3.f, 35.f, 24.f, FLinearColor::Transparent, FLinearColor(0.2f, 0.2f, 0.3f), FLinearColor(0.f, 1.f, 1.f, 1.f)))
		{
			// Cycle Camera views
			APawn* playerPawn = PC->GetPawn();
			if (playerPawn)
			{
				// Simply toggle camera height or position procedurally inside the project
			}
		}
		// Divider 2
		DrawRect(FLinearColor(0.5f, 0.5f, 0.5f, 0.3f), barX + 92.f, barY + 8.f, 1.f, 14.f);

		// Button: MUSIC
		static bool bMusicActive = true;
		FString musicSymbol = bMusicActive ? TEXT("🔊") : TEXT("🔇");
		if (DrawButton(musicSymbol, barX + 99.f, barY + 3.f, 35.f, 24.f, FLinearColor::Transparent, FLinearColor(0.2f, 0.2f, 0.3f), FLinearColor::Green))
		{
			bMusicActive = !bMusicActive;
			GameMode->PlaySynthSound(TEXT("toggle_music"));
		}

		// Divider 3
		DrawRect(FLinearColor(0.5f, 0.5f, 0.5f, 0.3f), barX + 139.f, barY + 8.f, 1.f, 14.f);

		// Button: BOMB (Mobile touch trigger HUD button!)
		if (DrawButton(TEXT("💣"), barX + 146.f, barY + 3.f, 44.f, 24.f, FLinearColor::Transparent, FLinearColor(0.4f, 0.1f, 0.1f), FLinearColor(1.0f, 0.3f, 0.43f)))
		{
			if (Rocket) Rocket->TriggerBomb();
		}

		// Boss Battle Warning HUD overlay
		if (GameMode->bIsBossBattle)
		{
			DrawText(TEXT("⚠️ ALIEN BOSS APPROACHING ⚠️"), FLinearColor::Red, Canvas->SizeX * 0.5f - 140.f, 100.f, nullptr, 1.4f);
		}
	}

	bWasPressedLastFrame = bIsPressed;
}

void ARocketDodgerHUD::DrawWindowOverlay(const FString& Title, const FString& Subtitle)
{
	// Helper to draw clean retro panel backdrop
}

bool ARocketDodgerHUD::DrawButton(const FString& Text, float X, float Y, float Width, float Height, FLinearColor BaseColor, FLinearColor HoverColor, FLinearColor TextColor)
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return false;

	float MouseX = 0.f;
	float MouseY = 0.f;
	PC->GetMousePosition(MouseX, MouseY);

	bool bHovered = (MouseX >= X && MouseX <= X + Width && MouseY >= Y && MouseY <= Y + Height);
	FLinearColor ButtonColor = bHovered ? HoverColor : BaseColor;

	// Draw button background (glassmorphic dark base)
	DrawRect(ButtonColor, X, Y, Width, Height);

	// Draw elegant neon border outline
	FLinearColor BorderColor = bHovered ? FLinearColor(0.0f, 0.96f, 0.83f, 1.0f) : FLinearColor(0.28f, 0.79f, 0.89f, 0.5f);
	DrawRect(BorderColor, X, Y, Width, 2.f); // Top border
	DrawRect(BorderColor, X, Y + Height - 2.f, Width, 2.f); // Bottom border
	DrawRect(BorderColor, X, Y, 2.f, Height); // Left border
	DrawRect(BorderColor, X + Width - 2.f, Y, 2.f, Height); // Right border

	// Draw text in the center
	float TextW = 0.f;
	float TextH = 0.f;
	GetTextSize(Text, TextW, TextH, nullptr, 1.0f);
	DrawText(Text, TextColor, X + (Width - TextW) * 0.5f, Y + (Height - TextH) * 0.5f, nullptr, 1.0f);

	bool bIsPressed = PC->IsInputKeyDown(EKeys::LeftMouseButton);
	bool bClick = bIsPressed && !bWasPressedLastFrame;

	return bHovered && bClick;
}

void ARocketDodgerHUD::LoadLeaderboard()
{
	Leaderboard.Empty();

	// Default rankings
	Leaderboard.Add(FPilotRecord(TEXT("NIVAN"), 5000));
	Leaderboard.Add(FPilotRecord(TEXT("COSMIC"), 3200));
	Leaderboard.Add(FPilotRecord(TEXT("ROBO"), 2100));
	Leaderboard.Add(FPilotRecord(TEXT("PILOT"), 1200));
	Leaderboard.Add(FPilotRecord(TEXT("NOOB"), 350));
}

void ARocketDodgerHUD::SaveScoreToLeaderboard(FString Name, int32 Score)
{
	Leaderboard.Add(FPilotRecord(Name, Score));

	// Sort score descending
	Leaderboard.Sort([](const FPilotRecord& A, const FPilotRecord& B) {
		return A.Score > B.Score;
	});

	// Save data state locally using FFileHelper or save system if desired
}

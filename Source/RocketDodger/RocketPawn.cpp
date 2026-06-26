#include "RocketPawn.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "LaserActor.h"
#include "RocketDodgerGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

ARocketPawn::ARocketPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// 1. Box Collision Component
	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->SetBoxExtent(FVector(100.f, 100.f, 50.f));
	CollisionComponent->SetCollisionProfileName(TEXT("Player"));

	// 2. Rocket Body Mesh
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	RocketMesh->SetupAttachment(RootComponent);

	// Load built-in engine shapes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMeshAsset(TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (ConeMeshAsset.Succeeded())
	{
		RocketMesh->SetStaticMesh(ConeMeshAsset.Object);
		RocketMesh->SetRelativeRotation(FRotator(-90.f, 90.f, 0.f)); // Orient to point forward along positive Y
		RocketMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.5f));
	}

	// 3. Wings
	LeftWingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftWing"));
	LeftWingMesh->SetupAttachment(RocketMesh);
	RightWingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightWing"));
	RightWingMesh->SetupAttachment(RocketMesh);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BoxMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (BoxMeshAsset.Succeeded())
	{
		LeftWingMesh->SetStaticMesh(BoxMeshAsset.Object);
		LeftWingMesh->SetRelativeLocation(FVector(-80.f, 0.f, 0.f));
		LeftWingMesh->SetRelativeScale3D(FVector(0.8f, 0.2f, 0.1f));

		RightWingMesh->SetStaticMesh(BoxMeshAsset.Object);
		RightWingMesh->SetRelativeLocation(FVector(80.f, 0.f, 0.f));
		RightWingMesh->SetRelativeScale3D(FVector(0.8f, 0.2f, 0.1f));
	}

	// 4. Pilot Inside Cockpit
	PilotMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pilot"));
	PilotMesh->SetupAttachment(RocketMesh);

	// 5. Shield Bubble (starts hidden)
	ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldBubble"));
	ShieldMesh->SetupAttachment(RocketMesh);
	ShieldMesh->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshAsset.Succeeded())
	{
		ShieldMesh->SetStaticMesh(SphereMeshAsset.Object);
		ShieldMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.0f));
	}

	// 6. Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(RootComponent);
	FollowCamera->SetRelativeLocation(FVector(0.f, -700.f, 150.f)); // Follow behind the ship
	FollowCamera->SetRelativeRotation(FRotator(-6.f, 90.f, 0.f)); // Look forward (pitch -6, yaw 90)

	// Stats
	MoveSpeed = 1600.0f;
	BoundaryX = FVector2D(-600.0f, 600.0f);
	BoundaryY = FVector2D(-400.0f, 400.0f);

	bHasShield = false;
	TripleShotTimer = 0.0f;
	BombCount = 0;
	CurrentSkin = TEXT("classic");

	CurrentRightInput = 0.0f;
	CurrentUpInput = 0.0f;
	TargetRoll = 0.0f;
	CurrentRoll = 0.0f;
	TargetPitch = 0.0f;
	CurrentPitch = 0.0f;
}

void ARocketPawn::BeginPlay()
{
	Super::BeginPlay();

	// Initialize dynamic material instances for vibrant arcade aesthetics
	if (RocketMesh)
	{
		UMaterialInstanceDynamic* DynRocket = RocketMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (DynRocket)
		{
			DynRocket->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.8f, 0.05f, 0.05f, 1.0f));
			DynRocket->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.8f, 0.05f, 0.05f, 1.0f));
		}
	}
	if (LeftWingMesh)
	{
		UMaterialInstanceDynamic* DynLeft = LeftWingMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (DynLeft)
		{
			DynLeft->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.1f, 0.1f, 0.15f, 1.0f));
			DynLeft->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.1f, 0.1f, 0.15f, 1.0f));
		}
	}
	if (RightWingMesh)
	{
		UMaterialInstanceDynamic* DynRight = RightWingMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (DynRight)
		{
			DynRight->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.1f, 0.1f, 0.15f, 1.0f));
			DynRight->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.1f, 0.1f, 0.15f, 1.0f));
		}
	}
	if (ShieldMesh)
	{
		UMaterialInstanceDynamic* DynShield = ShieldMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (DynShield)
		{
			DynShield->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.0f, 0.5f, 1.0f, 0.3f));
			DynShield->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.0f, 0.5f, 1.0f, 0.3f));
		}
	}
	if (PilotMesh)
	{
		UMaterialInstanceDynamic* DynPilot = PilotMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (DynPilot)
		{
			DynPilot->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.0f, 0.8f, 1.0f, 1.0f));
			DynPilot->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.0f, 0.8f, 1.0f, 1.0f));
		}
	}
}

void ARocketPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// --- 1. Movement Calculations ---
	FVector CurrentLocation = GetActorLocation();
	FVector Movement = FVector(CurrentRightInput, 0.f, CurrentUpInput) * MoveSpeed * DeltaTime; // Move in X (left/right) and Z (up/down)
	FVector NewLocation = CurrentLocation + Movement;

	// Clamp boundaries and lock Y to 0
	NewLocation.X = FMath::Clamp(NewLocation.X, BoundaryX.X, BoundaryX.Y);
	NewLocation.Y = 0.f;
	NewLocation.Z = FMath::Clamp(NewLocation.Z, BoundaryY.X, BoundaryY.Y);
	SetActorLocation(NewLocation);

	// --- 2. Bank / Tilt Rotation (Banking Roll & Pitch) ---
	TargetRoll = -CurrentRightInput * 25.0f; // Tilt left/right
	TargetPitch = CurrentUpInput * 15.0f;  // Tilt up/down

	CurrentRoll = FMath::FInterpTo(CurrentRoll, TargetRoll, DeltaTime, 10.0f);
	CurrentPitch = FMath::FInterpTo(CurrentPitch, TargetPitch, DeltaTime, 10.0f);

	FRotator NewRotation = FRotator(CurrentPitch, 0.0f, CurrentRoll);
	SetActorRotation(NewRotation);

	// --- 3. Power-Up Timers ---
	if (TripleShotTimer > 0.0f)
	{
		TripleShotTimer -= DeltaTime;
	}
}

void ARocketPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveRight", this, &ARocketPawn::MoveRight);
	PlayerInputComponent->BindAxis("MoveUp", this, &ARocketPawn::MoveUp);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ARocketPawn::FireLaser);
	PlayerInputComponent->BindAction("Bomb", IE_Pressed, this, &ARocketPawn::TriggerBomb);
}

void ARocketPawn::MoveRight(float Value)
{
	CurrentRightInput = Value;
}

void ARocketPawn::MoveUp(float Value)
{
	CurrentUpInput = Value;
}

void ARocketPawn::FireLaser()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FVector SpawnLoc = GetActorLocation() + FVector(0.f, 150.f, 0.f); // Fire slightly forward
	FRotator SpawnRot = FRotator::ZeroRotator;

	if (TripleShotTimer > 0.0f)
	{
		// Fire central, left-angle, and right-angle lasers
		World->SpawnActor<ALaserActor>(ALaserActor::StaticClass(), SpawnLoc, SpawnRot);
		World->SpawnActor<ALaserActor>(ALaserActor::StaticClass(), SpawnLoc, FRotator(0.f, -15.f, 0.f));
		World->SpawnActor<ALaserActor>(ALaserActor::StaticClass(), SpawnLoc, FRotator(0.f, 15.f, 0.f));
	}
	else
	{
		// Fire single standard laser
		World->SpawnActor<ALaserActor>(ALaserActor::StaticClass(), SpawnLoc, SpawnRot);
	}

	// Play laser sound synthesizer or wave file
	ARocketDodgerGameMode* GameMode = Cast<ARocketDodgerGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->PlaySynthSound(TEXT("laser"));
	}
}

void ARocketPawn::TriggerBomb()
{
	if (BombCount <= 0) return;

	BombCount--;

	ARocketDodgerGameMode* GameMode = Cast<ARocketDodgerGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->TriggerBombShockwave(GetActorLocation());
		GameMode->PlaySynthSound(TEXT("bomb_wave"));
	}
}

void ARocketPawn::CollectShield()
{
	bHasShield = true;
	if (ShieldMesh)
	{
		ShieldMesh->SetVisibility(true);
	}
}

void ARocketPawn::CollectTripleShot()
{
	TripleShotTimer = 5.0f; // 5 seconds of active spread fire
}

void ARocketPawn::CollectBomb()
{
	BombCount++;
	ARocketDodgerGameMode* GameMode = Cast<ARocketDodgerGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->UpdateBombHUD();
	}
}

void ARocketPawn::ApplySkin(FString SkinName)
{
	CurrentSkin = SkinName;
	if (!RocketMesh) return;
	UMaterialInstanceDynamic* DynRocket = RocketMesh->CreateAndSetMaterialInstanceDynamic(0);
	if (!DynRocket) return;

	FLinearColor SkinColor(0.8f, 0.05f, 0.05f, 1.0f); // Classic Red
	if (SkinName == TEXT("Neon Viper")) SkinColor = FLinearColor(0.0f, 0.9f, 0.2f, 1.0f);
	else if (SkinName == TEXT("Cosmic Shadow")) SkinColor = FLinearColor(0.4f, 0.0f, 0.8f, 1.0f);
	else if (SkinName == TEXT("Golden Legend")) SkinColor = FLinearColor(1.0f, 0.8f, 0.1f, 1.0f);

	DynRocket->SetVectorParameterValue(TEXT("Color"), SkinColor);
	DynRocket->SetVectorParameterValue(TEXT("BaseColor"), SkinColor);
}


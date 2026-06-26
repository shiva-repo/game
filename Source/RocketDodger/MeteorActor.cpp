#include "MeteorActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "RocketPawn.h"
#include "RocketDodgerGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

AMeteorActor::AMeteorActor()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->InitSphereRadius(80.f);
	CollisionComponent->SetCollisionProfileName(TEXT("Enemy"));

	MeteorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeteorMesh"));
	MeteorMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshAsset.Succeeded())
	{
		MeteorMesh->SetStaticMesh(SphereMeshAsset.Object);
		MeteorMesh->SetRelativeScale3D(FVector(1.6f, 1.6f, 1.6f));
	}

	ObstacleType = TEXT("normal");
	MaxHealth = 1;
	Health = 1;
	BaseSpeed = 400.f;
	DiagonalSpeedX = 0.f;
	SpinRate = 50.f;

	BossDirection = 1.0f;
	BossFireTimer = 0.0f;

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMeteorActor::OnOverlapBegin);
}

void AMeteorActor::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;

	if (MeteorMesh)
	{
		UMaterialInstanceDynamic* DynMeteor = MeteorMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (DynMeteor)
		{
			FLinearColor MatColor(0.25f, 0.2f, 0.2f, 1.0f); // Normal rocky asteroid
			if (ObstacleType == TEXT("splitter")) MatColor = FLinearColor(0.0f, 0.8f, 1.0f, 1.0f); // Cyan
			else if (ObstacleType == TEXT("spacejunk")) MatColor = FLinearColor(0.15f, 0.3f, 0.6f, 1.0f); // Steel blue
			else if (ObstacleType == TEXT("boss")) MatColor = FLinearColor(0.8f, 0.1f, 0.8f, 1.0f); // Magenta Boss
			else if (ObstacleType == TEXT("star")) MatColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f); // Bright star

			DynMeteor->SetVectorParameterValue(TEXT("Color"), MatColor);
			DynMeteor->SetVectorParameterValue(TEXT("BaseColor"), MatColor);
		}
	}

	if (ObstacleType == TEXT("star"))
	{
		if (CollisionComponent)
		{
			CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		SetActorScale3D(FVector(0.15f, 0.15f, 0.15f));
	}
}

void AMeteorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Rotate slowly to simulate spin in space
	AddActorLocalRotation(FRotator(SpinRate * DeltaTime, SpinRate * 0.5f * DeltaTime, 0.f));

	if (ObstacleType == TEXT("boss"))
	{
		// --- BOSS MOVEMENT & MECHANICS ---
		FVector Loc = GetActorLocation();

		// Move down to entrance height, then hover left/right
		if (Loc.Y > 2200.0f)
		{
			Loc.Y -= BaseSpeed * DeltaTime;
		}
		else
		{
			Loc.X += BossDirection * 350.0f * DeltaTime;
			if (Loc.X > 450.f)
			{
				Loc.X = 450.f;
				BossDirection = -1.0f;
			}
			else if (Loc.X < -450.f)
			{
				Loc.X = -450.f;
				BossDirection = 1.0f;
			}

			// Fire plasma bolts
			BossFireTimer += DeltaTime;
			if (BossFireTimer >= 1.5f)
			{
				BossFireTimer = 0.f;
				FireBossPlasmaBolt();
			}
		}
		SetActorLocation(Loc);
	}
	else
	{
		// --- REGULAR METEOR MOVEMENT ---
		FVector Loc = GetActorLocation();
		Loc.Y -= BaseSpeed * DeltaTime; // move towards player
		Loc.X += DiagonalSpeedX * DeltaTime; // diagonal split path
		SetActorLocation(Loc);

		// Destroy if past the player
		if (Loc.Y < -500.f)
		{
			Destroy();
		}
	}
}

void AMeteorActor::ApplyMeteorDamage(int32 Amount)
{
	Health -= Amount;

	// Flash material in blueprint or play impact sound
	ARocketDodgerGameMode* GameMode = Cast<ARocketDodgerGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->PlaySynthSound(TEXT("impact"));
	}

	if (Health <= 0)
	{
		if (ObstacleType == TEXT("splitter"))
		{
			SpawnSplitChildren();
		}

		if (GameMode)
		{
			if (ObstacleType == TEXT("boss"))
			{
				GameMode->OnBossDefeated();
			}
			else
			{
				int32 Points = (ObstacleType == TEXT("spacejunk")) ? 50 : 20;
				GameMode->AwardScore(Points);
			}
		}

		Destroy();
	}
}

void AMeteorActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this))
	{
		ARocketPawn* Rocket = Cast<ARocketPawn>(OtherActor);
		if (Rocket)
		{
			if (Rocket->bHasShield)
			{
				Rocket->bHasShield = false;
				if (Rocket->ShieldMesh)
				{
					Rocket->ShieldMesh->SetVisibility(false);
				}
				ARocketDodgerGameMode* GameMode = Cast<ARocketDodgerGameMode>(UGameplayStatics::GetGameMode(this));
				if (GameMode)
				{
					GameMode->PlaySynthSound(TEXT("shield_break"));
				}
			}
			else
			{
				ARocketDodgerGameMode* GameMode = Cast<ARocketDodgerGameMode>(UGameplayStatics::GetGameMode(this));
				if (GameMode)
				{
					GameMode->ReduceLives();
				}
			}

			// Don't destroy Boss on player impact
			if (ObstacleType != TEXT("boss"))
			{
				Destroy();
			}
		}
	}
}

void AMeteorActor::SpawnSplitChildren()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FVector Loc = GetActorLocation();

	// Spawn child 1 (moving diagonally left)
	AMeteorActor* Child1 = World->SpawnActor<AMeteorActor>(AMeteorActor::StaticClass(), Loc, FRotator::ZeroRotator);
	if (Child1)
	{
		Child1->ObstacleType = TEXT("normal");
		Child1->BaseSpeed = BaseSpeed;
		Child1->DiagonalSpeedX = -150.f;
		Child1->SetActorScale3D(FVector(0.5f));
	}

	// Spawn child 2 (moving diagonally right)
	AMeteorActor* Child2 = World->SpawnActor<AMeteorActor>(AMeteorActor::StaticClass(), Loc, FRotator::ZeroRotator);
	if (Child2)
	{
		Child2->ObstacleType = TEXT("normal");
		Child2->BaseSpeed = BaseSpeed;
		Child2->DiagonalSpeedX = 150.f;
		Child2->SetActorScale3D(FVector(0.5f));
	}
}

void AMeteorActor::FireBossPlasmaBolt()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FVector Loc = GetActorLocation() - FVector(0.f, 150.f, 0.f); // spawn slightly in front of boss

	// Spawn plasma bolts that move down along negative Y
	AMeteorActor* Bolt = World->SpawnActor<AMeteorActor>(AMeteorActor::StaticClass(), Loc, FRotator::ZeroRotator);
	if (Bolt)
	{
		Bolt->ObstacleType = TEXT("normal");
		Bolt->BaseSpeed = BaseSpeed + 200.f; // plasma is fast
		Bolt->SetActorScale3D(FVector(0.4f));
		// Change material to bright magenta in blueprint
	}

	ARocketDodgerGameMode* GameMode = Cast<ARocketDodgerGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->PlaySynthSound(TEXT("plasma_fire"));
	}
}

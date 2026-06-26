#include "PowerupActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "RocketPawn.h"
#include "RocketDodgerGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

APowerupActor::APowerupActor()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->InitSphereRadius(40.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	PowerupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PowerupMesh"));
	PowerupMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshAsset.Succeeded())
	{
		PowerupMesh->SetStaticMesh(SphereMeshAsset.Object);
		PowerupMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.8f));
	}

	// Explicitly assign BasicShapeMaterial to replace the default beige checkerboard WorldGridMaterial!
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMatAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BaseMatAsset.Succeeded())
	{
		PowerupMesh->SetMaterial(0, BaseMatAsset.Object);
	}

	PowerupType = TEXT("shield");
	FallSpeed = 300.0f;
	SpinSpeed = 100.0f;

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &APowerupActor::OnOverlapBegin);
}

void APowerupActor::BeginPlay()
{
	Super::BeginPlay();

	if (PowerupMesh)
	{
		UMaterialInstanceDynamic* DynPowerup = PowerupMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (DynPowerup)
		{
			FLinearColor MatColor(0.0f, 0.5f, 1.0f, 1.0f); // Shield blue
			if (PowerupType == TEXT("triple")) MatColor = FLinearColor(1.0f, 0.8f, 0.1f, 1.0f); // Gold
			else if (PowerupType == TEXT("bomb")) MatColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f); // Red

			DynPowerup->SetVectorParameterValue(TEXT("Color"), MatColor);
			DynPowerup->SetVectorParameterValue(TEXT("BaseColor"), MatColor);
			DynPowerup->SetVectorParameterValue(TEXT("EmissiveColor"), MatColor);
			DynPowerup->SetVectorParameterValue(TEXT("Emissive"), MatColor);
			DynPowerup->SetVectorParameterValue(TEXT("Tint"), MatColor);
		}
	}
}

void APowerupActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Float down screen towards player (negative Y)
	FVector Loc = GetActorLocation();
	Loc.Y -= FallSpeed * DeltaTime;
	SetActorLocation(Loc);

	// Rotate item
	AddActorLocalRotation(FRotator(0.f, SpinSpeed * DeltaTime, 0.f));

	// Clean up if it floats past player
	if (Loc.Y < -500.0f)
	{
		Destroy();
	}
}

void APowerupActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this))
	{
		ARocketPawn* Rocket = Cast<ARocketPawn>(OtherActor);
		if (Rocket)
		{
			if (PowerupType == TEXT("shield"))
			{
				Rocket->CollectShield();
			}
			else if (PowerupType == TEXT("triple"))
			{
				Rocket->CollectTripleShot();
			}
			else if (PowerupType == TEXT("bomb"))
			{
				Rocket->CollectBomb();
			}

			// Play collection sound
			ARocketDodgerGameMode* GameMode = Cast<ARocketDodgerGameMode>(UGameplayStatics::GetGameMode(this));
			if (GameMode)
			{
				GameMode->PlaySynthSound(TEXT("powerup"));
			}

			Destroy(); // Consume item
		}
	}
}

#include "LaserActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "MeteorActor.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

ALaserActor::ALaserActor()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->InitSphereRadius(25.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));

	LaserMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaserMesh"));
	LaserMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMeshAsset.Succeeded())
	{
		LaserMesh->SetStaticMesh(CylinderMeshAsset.Object);
		LaserMesh->SetRelativeRotation(FRotator(90.f, 0.f, 0.f)); // Orient horizontally along positive Y
		LaserMesh->SetRelativeScale3D(FVector(0.2f, 0.2f, 1.5f));
	}

	LaserSpeed = 2500.0f;
	MaxYDepth = 3500.0f;

	// Bind overlap event
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ALaserActor::OnOverlapBegin);
}

void ALaserActor::BeginPlay()
{
	Super::BeginPlay();

	if (LaserMesh)
	{
		UMaterialInstanceDynamic* DynLaser = LaserMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (DynLaser)
		{
			FLinearColor LaserColor(0.0f, 1.0f, 0.2f, 1.0f); // Bright neon green/cyan
			DynLaser->SetVectorParameterValue(TEXT("Color"), LaserColor);
			DynLaser->SetVectorParameterValue(TEXT("BaseColor"), LaserColor);
		}
	}
}

void ALaserActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Move forward along Y axis
	FVector NewLocation = GetActorLocation() + FVector(0.f, LaserSpeed * DeltaTime, 0.f);
	SetActorLocation(NewLocation);

	// Destroy if it goes out of range
	if (GetActorLocation().Y > MaxYDepth)
	{
		Destroy();
	}
}

void ALaserActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this))
	{
		AMeteorActor* Meteor = Cast<AMeteorActor>(OtherActor);
		if (Meteor)
		{
			// Damage meteor
			Meteor->ApplyMeteorDamage(1);
			Destroy(); // Destroy laser
		}
	}
}

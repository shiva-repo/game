#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaserActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class ROCKETDODGER_API ALaserActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ALaserActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* LaserMesh;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float LaserSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxYDepth;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeteorActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class ROCKETDODGER_API AMeteorActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AMeteorActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeteorMesh;

	// Obstacle Stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FString ObstacleType; // "normal", "splitter", "spacejunk", "boss"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DiagonalSpeedX; // For split children moving diagonally

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float SpinRate;

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void ApplyMeteorDamage(int32 Amount);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	float BossDirection;
	float BossFireTimer;
	void SpawnSplitChildren();
	void FireBossPlasmaBolt();
};

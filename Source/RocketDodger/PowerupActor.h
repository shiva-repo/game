#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerupActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class ROCKETDODGER_API APowerupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APowerupActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* PowerupMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FString PowerupType; // "shield", "triple", "bomb"

	UPROPERTY(EditAnywhere, Category = "Movement")
	float FallSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float SpinSpeed;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "RocketPawn.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class ALaserActor;

UCLASS()
class ROCKETDODGER_API ARocketPawn : public APawn
{
	GENERATED_BODY()

public:
	ARocketPawn();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* RocketMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* LeftWingMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* RightWingMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* PilotMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ShieldMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* FollowCamera;

	// Movement Settings
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	FVector2D BoundaryX; // Min/Max X boundaries

	UPROPERTY(EditAnywhere, Category = "Movement")
	FVector2D BoundaryY; // Min/Max Y boundaries

	// Game States
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bHasShield;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	float TripleShotTimer;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	int32 BombCount;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	FString CurrentSkin;

	// Input handlers
	void MoveRight(float Value);
	void MoveUp(float Value);
	void FireLaser();
	void TriggerBomb();

	// Actions
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void CollectShield();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void CollectTripleShot();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void CollectBomb();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void ApplySkin(FString SkinName);

private:
	float CurrentRightInput;
	float CurrentUpInput;

	float TargetRoll;
	float CurrentRoll;
	float TargetPitch;
	float CurrentPitch;
};

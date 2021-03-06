// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VRCharacter.generated.h"

UCLASS()
class UNREALCPPVR_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
private:
	void MoveForward(float Throttle);
	void MoveRight(float Throttle);
	bool FindTeleportDestination(FVector &OutLocation);
	void UpdateTeleportDestination();
	void StartFade(float FromAlpha, float ToAlpha);
	void BeginTeleport();
	void EndTeleport();
	void UpdateBlinkers();
	FVector2D GetBlinkerCentre();

	UPROPERTY()
	class USceneComponent* VRRoot;
	UPROPERTY()
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere)
		class UMotionControllerComponent* LeftController;
	UPROPERTY(EditAnywhere)
		class UMotionControllerComponent* RightController;

	UPROPERTY()
		class UPostProcessComponent* PostProcessComponent;

	UPROPERTY()
		UMaterialInstanceDynamic* BlinkerMaterialInstance;
	

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* DestinationMarker;

	UPROPERTY(EditAnywhere, Category = "Setup")
		UMaterialInterface* BlinkerMaterialBase;

	UPROPERTY(EditAnywhere, Category = "Setup")
		float MaxTeleportDistance = 10000.f;

	UPROPERTY(EditAnywhere, Category = "Setup")
		float TeleportProjectileRadius = 10.f;

	UPROPERTY(EditAnywhere, Category = "Setup")
		float TeleportProjectileSpeed = 1000.f;


	UPROPERTY(EditAnywhere, Category = "Setup")
		float TeleportSimulationTime= 8.f;

	UPROPERTY(EditAnywhere, Category = "Setup")
		float TeleportFadeTime = 0.7;

	UPROPERTY(EditAnywhere, Category = "Setup")
		FVector TeleportProjectionExtent = FVector (100, 100, 100);
	UPROPERTY(EditAnywhere, Category = "Setup")
		UCurveFloat* RadiusVSVelocity;
};

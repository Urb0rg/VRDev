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

	void UpdateTeleportDestination();

	void BeginTeleport();
	void EndTeleport();

	UPROPERTY()
	class USceneComponent* VRRoot;
	UPROPERTY()
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* DestinationMarker;

	UPROPERTY(EditAnywhere, Category = "Setup")
		float MaxTeleportDistance = 10000.f;
	UPROPERTY(EditAnywhere, Category = "Setup")
		float TeleportFadeTime = 0.7;
};

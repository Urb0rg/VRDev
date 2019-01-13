// Fill out your copyright notice in the Description page of Project Settings.

#include "VRCharacter.h"
#include "Classes/Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Classes/Engine/World.h"
#include "Classes/Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "Classes/GameFramework/PlayerController.h"
#include "Public/TimerManager.h"
#include "Classes/GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PostProcessComponent.h"
#include "Components/CapsuleComponent.h"
#include "Public/DrawDebugHelpers.h"
#include "MotionControllerComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Classes/Components/InputComponent.h"
	

// Sets default values
AVRCharacter::AVRCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	VRRoot->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VRRoot);

	LeftController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LController"));
	LeftController->SetupAttachment(VRRoot);
	LeftController->SetTrackingSource(EControllerHand::Left);

	RightController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RController"));
	RightController->SetupAttachment(VRRoot);
	RightController->SetTrackingSource(EControllerHand::Right);

	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
	DestinationMarker->SetupAttachment(GetRootComponent());

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
	PostProcessComponent->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	DestinationMarker->SetVisibility(false);
	if (BlinkerMaterialBase != nullptr)
	{
		BlinkerMaterialInstance = UMaterialInstanceDynamic::Create(BlinkerMaterialBase, this);
		PostProcessComponent->AddOrUpdateBlendable(BlinkerMaterialInstance);

	}
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector NewCameraOffset = Camera->GetComponentLocation() - GetActorLocation();
	NewCameraOffset.Z = 0;
	AddActorWorldOffset(NewCameraOffset);
	VRRoot->AddWorldOffset(-NewCameraOffset);

	UpdateTeleportDestination();
	UpdateBlinkers();
}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(TEXT("Move Forward"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("Move Right"), this, &AVRCharacter::MoveRight);
	PlayerInputComponent->BindAction(TEXT("Teleport"), IE_Released, this, &AVRCharacter::BeginTeleport);
}

void AVRCharacter::MoveForward(float Throttle)
{
	AddMovementInput(Throttle * Camera->GetForwardVector());
	
}

void AVRCharacter::MoveRight(float Throttle)
{
	AddMovementInput(Throttle * Camera->GetRightVector());
}

bool AVRCharacter::FindTeleportDestination(FVector &OutLocation)
{
	
	auto Start = RightController->GetComponentLocation();
	auto Look = RightController->GetForwardVector();
	Look = Look.RotateAngleAxis(30, RightController->GetRightVector());
	auto End = Start + Look  * MaxTeleportDistance;  


	FPredictProjectilePathParams Params(TeleportProjectileRadius, Start,Look* TeleportProjectileSpeed,TeleportSimulationTime, ECollisionChannel::ECC_Visibility,this);
	FPredictProjectilePathResult Result;
	Params.DrawDebugType = EDrawDebugTrace::ForOneFrame;
	Params.bTraceComplex = true;

	bool bHit = UGameplayStatics::PredictProjectilePath(this, Params, Result);

	
	/*FHitResult TeleportDestination;
	bool Hit = GetWorld()->LineTraceSingleByChannel(TeleportDestination, Start, End, ECC_Visibility);
	*/

	if (!bHit) { return false; }

	FNavLocation NavLocation;	
	bool bOnNavMesh = UNavigationSystemV1::GetNavigationSystem(GetWorld())->ProjectPointToNavigation(Result.HitResult.Location, NavLocation, TeleportProjectionExtent);

	if (!bOnNavMesh) { return false; }

	OutLocation = NavLocation.Location;
	return bHit && bOnNavMesh;
}


void  AVRCharacter::UpdateTeleportDestination()
{
	FVector Location;
	bool bHasDestination = FindTeleportDestination(Location);
	

		
	if (bHasDestination)
	{
		DestinationMarker->SetWorldLocation(Location); 

		DestinationMarker->SetVisibility(true);
	}
	else { DestinationMarker->SetVisibility(false); }

	return;
}

void AVRCharacter::UpdateBlinkers()
{
	if (!RadiusVSVelocity)return;

	float Speed = GetVelocity().Size();
	float Radius = RadiusVSVelocity->GetFloatValue(Speed);	

	BlinkerMaterialInstance->SetScalarParameterValue(TEXT("Radius"), Radius);

	//FVector2D Centre = GetBlinkerCentre();
	//BlinkerMaterialInstance->SetVectorParameterValue(TEXT("Centre"), FLinearColor(Centre.X, Centre.Y, 0 ));	
}

FVector2D AVRCharacter::GetBlinkerCentre()
{
	FVector MovementDirection = GetVelocity().GetSafeNormal();
	if (MovementDirection.IsNearlyZero())
	{
		return FVector2D(0.5, 0.5);
	}

	FVector WorldStationaryLocation;
	if (FVector::DotProduct(Camera->GetForwardVector(), MovementDirection) > 0)
	{
		FVector WorldStationaryLocation = Camera->GetComponentLocation() + MovementDirection * 1000;
	}
	else
	{
		FVector WorldStationaryLocation = Camera->GetComponentLocation() - MovementDirection * 1000;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return FVector2D(0.5, 0.5);
	}

	FVector2D ScreenStationaryLocation;
	PC->ProjectWorldLocationToScreen(WorldStationaryLocation, ScreenStationaryLocation);

	int32 SizeX, SizeY;
	PC->GetViewportSize(SizeX, SizeY);
	ScreenStationaryLocation.X /= SizeX;
	ScreenStationaryLocation.Y /= SizeY;

	return ScreenStationaryLocation;
}


void  AVRCharacter::BeginTeleport()
{
	StartFade(0, 1);
	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, this, &AVRCharacter::EndTeleport, TeleportFadeTime);
	return;
}

void AVRCharacter::EndTeleport()
{
	StartFade(1, 0);
	FVector Vec (0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	SetActorLocation(DestinationMarker->GetComponentLocation() + Vec);
	return;
}

void AVRCharacter::StartFade(float FromAlpha, float ToAlpha)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC != nullptr)
	{
		PC->PlayerCameraManager->StartCameraFade(FromAlpha, ToAlpha, TeleportFadeTime, FLinearColor::Black);
	}
	return;
}
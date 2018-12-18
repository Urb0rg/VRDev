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
#include "Components/CapsuleComponent.h"
#include "Public/DrawDebugHelpers.h"
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
	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
	DestinationMarker->SetupAttachment(GetRootComponent());

}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	DestinationMarker->SetVisibility(false);
	
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
	FHitResult TeleportDestination;
	auto Start = Camera->GetComponentLocation();
	auto End = Start + Camera->GetForwardVector() * MaxTeleportDistance;
	bool Hit = GetWorld()->LineTraceSingleByChannel(TeleportDestination, Start, End, ECC_Visibility);

	if (!Hit) { return false; }

	FNavLocation NavLocation;	

	bool bOnNavMesh = UNavigationSystemV1::GetNavigationSystem(GetWorld())->ProjectPointToNavigation(TeleportDestination.Location, NavLocation, TeleportProjectionExtent);

	if (!bOnNavMesh) { return false; }

	OutLocation = NavLocation.Location;
	return Hit && bOnNavMesh;
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
// Fill out your copyright notice in the Description page of Project Settings.

#include "VRCharacter.h"
#include "Classes/Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Classes/Engine/World.h"
#include "Components/StaticMeshComponent.h"
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

void  AVRCharacter::UpdateTeleportDestination()
{
	FHitResult TeleportDestination;
	auto Start = Camera->GetComponentLocation();
	auto End = Start + Camera->GetForwardVector() * MaxTeleportDistance;

	bool Hit = GetWorld()->LineTraceSingleByChannel(TeleportDestination, Start, End, ECC_Visibility);
	if (Hit)
	{
		DestinationMarker->SetWorldLocation(TeleportDestination.Location); 

		DestinationMarker->SetVisibility(true);
	}
	else { DestinationMarker->SetVisibility(false); }

	return;
}

void  AVRCharacter::BeginTeleport()
{}
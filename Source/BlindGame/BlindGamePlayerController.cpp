// Copyright Epic Games, Inc. All Rights Reserved.


#include "BlindGamePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"

void ABlindGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}
}

void ABlindGamePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ManageCameraShake();
}

void ABlindGamePlayerController::ManageCameraShake()
{
	if (!IdleCameraShake || !WalkingCameraShake || !GetPawn()) return;
	
	FVector Velocity = GetPawn()->GetVelocity();
	float Speed = Velocity.Size();

	if (Speed > 0)
	{
		ClientStartCameraShake(WalkingCameraShake);
	}else
	{
		ClientStartCameraShake(IdleCameraShake);
	}
}

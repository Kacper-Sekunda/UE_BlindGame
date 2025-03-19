// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "BlindGamePlayerController.generated.h"

class UInputMappingContext;

/**
 *
 */
UCLASS()
class BLINDGAME_API ABlindGamePlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Context to be used for player input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* InputMappingContext;

	virtual void BeginPlay() override;
	void Tick(float DeltaTime);
	void ManageCameraShake();
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> IdleCameraShake;

	UPROPERTY(EditDefaultsOnly, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> WalkingCameraShake;

	UPROPERTY()
	TSubclassOf<UCameraShakeBase> ActiveCameraShake;
};

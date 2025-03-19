// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "BlindGameCharacter.generated.h"

class USpringArmComponent;
class USpotLightComponent;
class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ABlindGameCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	

public:
	
	ABlindGameCharacter();

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	//Sounds
	UPROPERTY(EditAnywhere, Category = "Sounds")	//Hit sound
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = "Sounds")	//Slide sound
	USoundBase* WallSlideSound;

	//Flashlight
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight")
	USpringArmComponent* FlashlightSpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight")
	USpotLightComponent* Flashlight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* FlashlightAction;
	
	//DEPRECATED with OnHit
	//float LastHitTime = 0.0f;
	//const float HitCooldown = 0.5f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);
    
	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	//DEPRECATED -- Old method of doing hit sound without sliding
	/*UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);*/

	UPROPERTY()					//Slide audio component
	UAudioComponent* SlideAudioComponent;
	
	bool bIsMoving = false;		//Is moving
	bool bIsSliding = false;	//Is sliding
	bool bHasHitWall = false;	//Has hit wall
	
	float PlayerSpeed;			//Player speed
	
	bool bIsFlashlightOn;
	
	void CheckWallSlideSound();										//Check sound
	void StartWallSlideSound(const FHitResult& WallHit) const;		//Play wall slide sound
	void StopWallSlideSound() const;								//Stop wall slide sound
	void UpdateSlideSoundLocation(const FHitResult& WallHit) const;	//Update slide sound loc
	void ToggleFlashlightPressed();
	void ToggleFlashlightReleased();
};


// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlindGameCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/AudioComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

class UNiagaraSystem;
DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ABlindGameCharacter::ABlindGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	//TEMPLATE^^^

	FlashlightSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("FlashlightSpringArm"));
	FlashlightSpringArm->SetupAttachment(RootComponent);
	FlashlightSpringArm->TargetArmLength = 50.f;
	FlashlightSpringArm->bUsePawnControlRotation = true;

	Flashlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Flashlight"));
	Flashlight->SetupAttachment(FlashlightSpringArm);
	Flashlight->SetIntensity(5000.f);
	Flashlight->SetLightColor(FLinearColor::White);
	Flashlight->SetVisibility(false);
	bIsFlashlightOn = false;
	
	//GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ABlindGameCharacter::OnHit);
	//DEPRECATED -- Old method of doing hit sound without sliding

	SlideAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SlideAudioComponent"));
	SlideAudioComponent->bAutoActivate = false; // Don't play automatically
	SlideAudioComponent->SetupAttachment(RootComponent);
}

//TEMPLATE
void ABlindGameCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

void ABlindGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckWallSlideSound();
}

//TEMPLATE
void ABlindGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABlindGameCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABlindGameCharacter::Look);

		//Flashlight
		EnhancedInputComponent->BindAction(FlashlightAction, ETriggerEvent::Started, this,
		                                   &ABlindGameCharacter::ToggleFlashlightPressed);
		EnhancedInputComponent->BindAction(FlashlightAction, ETriggerEvent::Completed, this,
		                                   &ABlindGameCharacter::ToggleFlashlightReleased);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

//TEMPLATE
void ABlindGameCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

//TEMPLATE
void ABlindGameCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

//DEPRECATED -- Old method of doing hit sound without sliding
/*void ABlindGameCharacter::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this) return;
	if (!HitSound) return;
	
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastHitTime < HitCooldown) return;
	LastHitTime = CurrentTime;
	
	FVector Direction = Hit.Normal;
	if (FVector::DotProduct(Direction, GetActorRightVector()) < 0.0f) 
	{
		Direction = -Direction;
	}

	FVector SoundLocation = Hit.ImpactPoint + Direction;
	//UGameplayStatics::PlaySoundAtLocation(this, HitSound, SoundLocation);
}*/

// Hit and Sliding sound :0
void ABlindGameCharacter::CheckWallSlideSound()
{
	FVector PlayerVelocity = GetCharacterMovement()->Velocity;	//Get velocity
	PlayerSpeed = PlayerVelocity.Size();						//Get player speed
	bIsMoving = PlayerVelocity.Size() > 25.f;					//Is moving?
	
	FVector Start = GetActorLocation();	//Start of trace
	float TraceRadius = 75.f;			//Trace radius

	FCollisionQueryParams QueryParams;	//Collision Params
	QueryParams.AddIgnoredActor(this);	//Ignore self

	TArray<FHitResult> OutHits;			//Array
	
	bool bIsTouchingWall = GetWorld()->SweepMultiByChannel(
		OutHits,								//Output
		Start,										//Start of trace
		Start + FVector(0, 0, 1),	//End of trace
		FQuat::Identity,							//Rot?
		ECC_Visibility,								//Trace channel
		FCollisionShape::MakeSphere(TraceRadius),	//Shape and radius
		QueryParams									//Params
	);
	
	{
		/*DrawDebugSphere(GetWorld(),	//Debug Hit
	   Start,			//Start
	   TraceRadius,	//Radius
	   12,				//Segments
	   FColor::Red,	//Colour
	   false,			//Persistant?
	   0.0f			//Lifetime
	   );*/
	}	// Debug Sphere
	
	if (!SlideAudioComponent || !WallSlideSound || !HitSound) return; //Check valid
	
	if (bIsTouchingWall)	//Touching wall
	{
		UpdateSlideSoundLocation(OutHits[0]);
		
		if (!bHasHitWall)	//Not hit wall
		{
			bHasHitWall = true;

			FVector Direction = OutHits[0].Normal;  //Direction
			
			if (FVector::DotProduct(Direction, GetActorForwardVector()) < 0.75f)	//Dot product
			{
				Direction = -Direction;  //Flip direction if playing wrong way
			}
			
			FVector WallOffsetLocation = GetActorLocation() + Direction * 250.0f;		//Offset towards wall
			UGameplayStatics::PlaySoundAtLocation(this, HitSound,		//Hit Sound
				WallOffsetLocation);
		}

		if (bHasHitWall && bIsMoving)	//Hit wall and moving
		{
			if (!bIsSliding)			//Not sliding
			{
				bIsSliding = true;
				StartWallSlideSound(OutHits[0]);	//Play slide Sound
			}

			float MinSpeed = 50.f;	  // Min speed for pitch change
			float MaxSpeed = 1000.f;  // Max speed before clamping
			float MinPitch = 1.0f;    // Normal pitch
			float MaxPitch = 4.0f;    // Faster pitch

			float NewPitch = FMath::GetMappedRangeValueClamped(
			FVector2D(MinSpeed, MaxSpeed),	//Input Range
			FVector2D(MinPitch, MaxPitch),	//Output Range
			PlayerSpeed									//Speed :0
			);

			SlideAudioComponent->SetPitchMultiplier(NewPitch);
		}
		else						//Probably hit wall and not moving
		{
			StopWallSlideSound();	//Stop sliding sound
			bIsSliding = false;
		}
	}
	else							//Probably not touching wall
	{
		StopWallSlideSound();		//Stop sliding sound
		bHasHitWall = false;
		bIsSliding = false;
	}
}

//Plays sliding sound
void ABlindGameCharacter::StartWallSlideSound(const FHitResult& WallHit) const
{
	if (!WallSlideSound && !SlideAudioComponent) return;	//Check valid
	SlideAudioComponent->SetSound(WallSlideSound);			//Set sound
	SlideAudioComponent->Play();							//Play
}

//Stops sliding sound
void ABlindGameCharacter::StopWallSlideSound() const
{
	if (!SlideAudioComponent) return;	//Check valid
	SlideAudioComponent->Stop();		//Halt!
}

//Update sliding sound location
void ABlindGameCharacter::UpdateSlideSoundLocation(const FHitResult& WallHit) const
{
	if (!SlideAudioComponent) return;   //Check slide component
	
	FVector Direction = WallHit.Normal;	//Get direction of wall
	float ForwardDot = FVector::DotProduct(Direction, GetActorForwardVector());
	float RightDot = FVector::DotProduct(Direction, GetActorRightVector());
	if (ForwardDot < 1.f)
	{
		Direction = -Direction;			//Invert direction
	}

	FVector WallOffsetLocation = GetActorLocation() + Direction * 250.0f;	//Cal wall offset
	SlideAudioComponent->SetWorldLocation(WallOffsetLocation);				//Set audio comp location

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	//if (!FFE_Forward || !FFE_Back || !FFE_Right || !FFE_Left) return;		//Check valid
	if (PlayerController)
	{
		if (ForwardDot > 0.7f)												//Forward
		{
			if (!FFE_Forward) return;
			PlayerController->ClientPlayForceFeedback(FFE_Forward);
		}
		else if (ForwardDot < -0.7f)										//Back
		{
			if (!FFE_Back) return;
			PlayerController->ClientPlayForceFeedback(FFE_Back);
		}
		else if (RightDot > -0.7f)											//Right
		{
			if (!FFE_Right) return;
			PlayerController->ClientPlayForceFeedback(FFE_Right);
		}
		else if (RightDot < 0.7f)											//Left
		{
			if (!FFE_Left) return;
			PlayerController->ClientPlayForceFeedback(FFE_Left);
		}
	}
}


//Flashlight 'light' disabled for gameplay purposes
void ABlindGameCharacter::ToggleFlashlightPressed()
{
	bIsFlashlightOn = true;
	//Flashlight->SetVisibility(bIsFlashlightOn);
}

void ABlindGameCharacter::ToggleFlashlightReleased()
{
	bIsFlashlightOn = false;
	//Flashlight->SetVisibility(bIsFlashlightOn);
}

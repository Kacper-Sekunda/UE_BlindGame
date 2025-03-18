// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlindGameGameMode.h"
#include "BlindGameCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABlindGameGameMode::ABlindGameGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "PortalProjectGameMode.h"
#include "PortalProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

APortalProjectGameMode::APortalProjectGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}

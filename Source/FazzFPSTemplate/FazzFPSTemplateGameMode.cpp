// Copyright Epic Games, Inc. All Rights Reserved.

#include "FazzFPSTemplateGameMode.h"
#include "FazzFPSTemplateCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFazzFPSTemplateGameMode::AFazzFPSTemplateGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}

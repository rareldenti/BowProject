// Copyright Epic Games, Inc. All Rights Reserved.

#include "BowProjectGameMode.h"
#include "BowProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABowProjectGameMode::ABowProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

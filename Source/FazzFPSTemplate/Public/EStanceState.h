// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EStanceState.generated.h"

UENUM(BlueprintType)
enum EStanceState
{
	Stand,
	Crouch,
	Sprint, 
	Prone
};
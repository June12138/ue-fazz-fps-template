// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponSwayStruct.generated.h"

USTRUCT(BlueprintType)
struct FWeaponSwayStruct {
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sway") float SwayYawMultiplier = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sway") float SwayYawMax = 15;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sway") float SwayPitchMultiplier = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sway") float SwayPitchMax = 15;
};
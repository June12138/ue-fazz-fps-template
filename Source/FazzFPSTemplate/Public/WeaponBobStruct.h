// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBobStruct.generated.h"

USTRUCT(BlueprintType)
struct FWeaponBobStruct {
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobFrequencyMultiplier = 0.75;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobLongitudeZ = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobLongitudeY = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobPitch = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobYaw = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobNoiseFrequencyMultiplier = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobNoiseLongitude = 0.5;
};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponRecoilStruct.generated.h"

USTRUCT(BlueprintType)
struct FWeaponRecoilStruct {
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Recoil") FVector RecoilOffset = FVector(-8.f, 2.f, 0.f); //后座终止位置偏移
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Recoil") FVector RecoilOffsetJitter = FVector(3.f,3.f,3.f); //后座随机偏移 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Recoil") FVector RecoilRotationOffset = FVector(0.f,0.f,-3.f); //后座终止旋转偏移
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Recoil") FVector RecoilRotationOffsetJitter = FVector(2.f,2.f,2.f);//后座旋转随机偏移 
};
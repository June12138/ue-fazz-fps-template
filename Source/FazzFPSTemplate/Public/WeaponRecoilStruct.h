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
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Recoil") FVector RecoilGradualOffset = FVector(0.f,2.f,0.f);//后座渐进偏移 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Recoil") FRotator RecoilGradualRotationOffset = FRotator(-0.25,0.f,0.f);//后座旋转渐进偏移 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Recoil") float RecoilGradualOffsetInterpolationRate = 2.f; //渐进后座偏移插值速率
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Recoil") float RecoilGradualOffsetRecoverRate = 5.f; //渐进后座偏回复插值速率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Recoil", meta=(AllowAbstract="false")) TSubclassOf<UCameraShakeBase> RecoilCameraShakeClass;
};
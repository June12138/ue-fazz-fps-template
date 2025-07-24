// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "WeaponAnimComponent_CPP.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FAZZFPSTEMPLATE_API UWeaponAnimComponent_CPP : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponAnimComponent_CPP();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	float ElapsedTime = 0.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Roots") USceneComponent* WeaponRoot = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Roots") UCameraComponent* CameraRoot = nullptr;
	FVector StartLocation;
	FVector StartRotation;
	FVector Result;
	FVector RotationResult;
	FVector JitterVector(FVector Input, FVector Jitter);
	//后坐力相关
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil") float RecoilAnimTime = 0.2f; //后坐力动画时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil") UCurveFloat* RecoilCurve;	// 后坐力曲线
	float CurrentRecoilTime = 0.0f;
		//后坐力位置偏移
	FVector CurrentRecoilOffset;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Recoil") FVector RecoilOffset = FVector(-8.f, 2.f, 0.f); //后座终止位置偏移
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Recoil") FVector RecoilOffsetJitter = FVector(3.f,3.f,3.f); //后座随机偏移 
		//后坐力旋转偏移
	FVector CurrentRecoilRotationOffset;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Recoil") FVector RecoilRotationOffset = FVector(0.f,0.f,-3.f); //后座终止旋转偏移
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Recoil") FVector RecoilRotationOffsetJitter = FVector(2.f,2.f,2.f);//后座旋转随机偏移 
	void UpdateRecoilEnd();
	UFUNCTION(BlueprintCallable)
	void StartRecoilAnim();
	bool IsPlayingRecoilAnim = false;
	UFUNCTION(BlueprintCallable) void Init(USceneComponent* WeaponRootToSet, UCameraComponent* CameraRootToSet);
	//武器晃动相关
	float BobFrequencyMultiplierCurrent = 0.75;
	float BobLongitudeCurrentZ = 1.f;
	float BobLongitudeCurrentY = 0.f;
	float BobNoiseCurrent = 0.5;
	FVector CurrentBobResult;
	FVector BobResult;
	void UpdateBob();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobInterpolationRate = 5;
		//Idle晃动
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobFrequencyMultiplierIdle = 0.75;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobLongitudeIdleZ = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobLongitudeIdleY = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobNoiseIdle = 0.5;
		//走路晃动
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobFrequencyMultiplierWalk = 4;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobLongitudeWalkZ = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobLongitudeWalkY = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobNoiseWalk = 0.7;
	FVector InputVector;
	UFUNCTION(BlueprintCallable)
	void SetInputVector(FVector Vector);
};

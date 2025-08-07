// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "WeaponBobStruct.h"
#include "WeaponSwayStruct.h"
#include "WeaponRecoilStruct.h"
#include "CPP_WeaponAnimComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class FAZZFPSTEMPLATE_API UCPP_WeaponAnimComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCPP_WeaponAnimComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
private:
	float SqrtAlpha(float DeltaTime, float Rate);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	float ElapsedTime = 0.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) bool ShouldPlayAnimation = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Roots") USceneComponent* WeaponRoot = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Roots") USceneComponent* Sight = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Roots") USceneComponent* CameraRoot = nullptr;
	UFUNCTION(BlueprintCallable) void Init(USceneComponent* WeaponRootToSet, USceneComponent* SightToSet, USceneComponent* CameraRootToSet);
	APlayerController* Controller = nullptr;
	void TrySetController();
	FVector CamInitialLocation;
	void UpdateSettings();
	//设置玩家输入数据
	UFUNCTION(BlueprintCallable) void SetInputVector(FVector Vector);
	UFUNCTION(BlueprintCallable) void SetInputRotator(FRotator Rotator);
	FVector2D InputVector2D;
	FVector InputVector;
	FRotator InputRotator;
	float MoveSize;
	//基准
	enum class EStanceState
	{
		Default,
		Sprint,
		Crouch
	};
	EStanceState CurrentStance = EStanceState::Default;
	FVector DefaultBaseLocation;
	FRotator DefaultBaseRotation;
		// 奔跑基准参数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base") FVector SprintBaseLocation = FVector(25, -1, -13);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base") FRotator SprintBaseRotation = FRotator(-19,-35,-24);
	UFUNCTION(BlueprintCallable) void StartSprint();
	UFUNCTION(BlueprintCallable) void EndSprint();
		// 下蹲基准参数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base") FVector CrouchBaseLocation = FVector(25,9,-8);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base") FRotator CrouchBaseRotation = FRotator(-2,3,-14);
	UFUNCTION(BlueprintCallable) void StartCrouch();
	UFUNCTION(BlueprintCallable) void EndCrouch();
		//当前基准参数
	FVector* TargetBaseLocation = &DefaultBaseLocation;
	FRotator* TargetBaseRotation = &DefaultBaseRotation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base") float BaseLocationInterpolationRate = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base") float BaseRotationInterpolationRate = 5.f;
	FVector CurrentBaseLocation;
	FRotator CurrentBaseRotation;
	//结算
	FVector Result;
	FRotator RotationResult;
	FVector JitterVector(FVector Input, FVector Jitter);
	//后坐力相关
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil") float RecoilAnimTime = 0.2f; //后坐力动画时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil") UCurveFloat* RecoilCurve;	// 后坐力曲线
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil") UForceFeedbackEffect* RecoilForceFeedbackEffect; //后坐力震动
	float CurrentRecoilTime = 0.0f;
		//后坐力位置偏移
	FVector RecoilTargetOffset;
	FVector GradualRecoilOffsetTarget;
	UPROPERTY(BlueprintReadOnly) FVector CurrentRecoilOffset; // ADS状态下这个变量会影响到准星偏移。万一要用这个数据读取准星，让蓝图能获取到当前后坐力偏移
	FRotator RecoilRotationResult = FRotator::ZeroRotator; //后坐力旋转偏移
	UPROPERTY(BlueprintReadOnly) FVector CurrentRecoilGradualOffset; //后坐力渐进偏移
	FVector RecoilRotationTargetOffset; //后坐力旋转偏移
	FRotator CurrentRecoilGradualRotOffset; //后坐力旋转渐进偏移
		//后坐力结构体
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Recoil") 
	FWeaponRecoilStruct DefaultRecoilStruct = FWeaponRecoilStruct{
		FVector(-1.f, 0.f, 0.f), //后座终止位置偏移
		FVector(1.f,1.f,0.5), //后座随机偏移
		FVector(1.f,0.f,0.f), //后座终止旋转偏移
		FVector(1.f,2.f,1.f), //后座随机旋转偏移
		FVector(-3.f,0.f,3.f), //后座旋转随机偏移 
		FRotator(6,0.f,0.f), //后座旋转渐进偏移 
		2.f, //后座旋转随机偏移插值速率
		5.f
	};
		//ADS后坐力结构体
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Recoil") 
	FWeaponRecoilStruct ADSRecoilStruct = FWeaponRecoilStruct{
		FVector(-0.25, 0.f, 0.3), //后座终止位置偏移
		FVector(0.1,0.05,0.f), //后座随机偏移
		FVector(0.3,0.0,0.0), //后座终止旋转偏移
		FVector(0.2,0.2,5.f), //后座随机旋转偏移
		FVector(0.f,0.f,3.f),//后座旋转随机偏移 
		FRotator(4.f,0.f,0.f), //后座旋转渐进偏移 
		2.f, //后座旋转随机偏移插值速率
		5.f
	}; 
	FWeaponRecoilStruct* CurrentRecoilStruct = &DefaultRecoilStruct;
	void UpdateRecoilEnd();
	void UpdateRecoil(float DeltaTime);
	UFUNCTION(BlueprintCallable)
	void StartRecoilAnim();
	bool IsPlayingRecoilAnim = false;
	//武器晃动相关
	FVector CurrentBobResult;
	FVector BobResult;
	FRotator CurrentBobResultRot;
	FRotator BobResultRot;
	void UpdateBob();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobInterpolationRate = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobRotationInterpolationRate = 3;
		//Idle晃动
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") FWeaponBobStruct IdleBob = FWeaponBobStruct{0.75, 1.f, 0.f, 3, 0, 0.5};
		//Walk晃动
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") FWeaponBobStruct WalkBob = FWeaponBobStruct{4, 3, 3, 3, 3, 0.7};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float CrouchMultiplier = 0.7; //下蹲时的晃动倍率
		//Run晃动
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") FWeaponBobStruct RunBob = FWeaponBobStruct{8, 3, 3, 3, 3, 0.7};
		//ADS Idle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") FWeaponBobStruct IdleBobADS = FWeaponBobStruct{1, 0.f, 0.f, 0.05, 0, 0.15};
		//ADS Walk晃动
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") FWeaponBobStruct WalkBobADS = FWeaponBobStruct{4, 0.f, 0.f, 0.1, 0.2, 0.15};
		FWeaponBobStruct* CurrentBob = &IdleBob;
	// Sway相关
	FRotator CurrentSway = FRotator::ZeroRotator;
	FRotator TargetSway = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sway") float SwayInterpolationRate = 5;
		// 默认姿态下sway
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sway")FWeaponSwayStruct DefaultSway = FWeaponSwayStruct{5, 15, 5, 15};
	FWeaponSwayStruct* CurrentSwayStruct = &DefaultSway;
		// 开镜状态下sway
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sway")FWeaponSwayStruct ADSSway = FWeaponSwayStruct{3, 0.3, 3, 0.3};
	void UpdateSway();
	// MovementOffset相关
	FVector CurrentMovementOffset = FVector::ZeroVector;
	FVector TargetMovementOffset = FVector::ZeroVector;
	float CurrentMovementRotationOffset = 0.f;
	float TargetMovementRotationOffset = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovementOffset") float MovementOffsetInterpolationRate = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovementOffset") FVector MovementOffsetMax = FVector(2.f, 2.f, 5.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovementOffset") float MovementRotationOffsetMax = 5.f;
	void UpdateMovementOffset();
	// 跳跃相关
	enum class EJumpState
	{
		Default,
		Start,
		MidAir,
		Land
	};
	EJumpState CurrentJumpState = EJumpState::Default;
	float CurrentJumpOffset = 0.f;
	float DefaultJumpOffset = 0.f;
	float* TargetJumpOffset = &DefaultJumpOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump") float JumpOffset = 8.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump") float MidAirOffset = 3.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump") float LandOffset = -3.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump") float JumpOffsetInterpolationRateUp = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump") float JumpOffsetInterpolationRateDown = 15.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump") float JumpTransitionTolerance = 0.7f; // 跳跃动画转换阶段时允许的误差范围，值越大转换越早
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump") float JumpADSMultiplier = 0.3f; 
	float* CurrentJumpInterpolationRate = &JumpOffsetInterpolationRateUp;
	UFUNCTION(BlueprintCallable) void StartJump();
	UFUNCTION(BlueprintCallable) void MidAir();
	UFUNCTION(BlueprintCallable) void EndJump();
	void UpdateJumpState(float Multiplier);
	void UpdateJump(float DeltaTime);
	// 侧头相关
	FVector CurrentTiltOffset = FVector::ZeroVector;
	UPROPERTY(BlueprintReadonly) float CurrentTiltRoll = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tilt") float TiltInterpolationRate = 5.f; //侧头插值速率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tilt") FVector TiltOffsetLeft = FVector(0.f, -20.f, 0.f); //左侧头偏移
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tilt") FVector TiltOffsetRight = FVector(0.f, 20.f, 0.f); //右侧头偏移
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tilt") float TiltRoll = 15.f;
	UPROPERTY(BlueprintReadonly) int TiltDirection = 0; // -1为左侧头，1为右侧头，0为无侧头
	UFUNCTION(BlueprintCallable) void SetTilt(int Direction);
	void UpdateTilt(float DeltaTime);
	// ADS相关
	bool ToADS = false;
	bool PlayingADSAnimation = false;
	bool IsAiming = false;
	FVector CurrentADSCorrection;
	FVector TargetADSCorrection = FVector::ZeroVector;
	FVector CurrentADSOffset = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS") FRotator ADSBaseRotation = FRotator(0.f, 0.f, 0.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS") UCurveFloat* ADSCurve;
	UFUNCTION(BlueprintCallable) void StartADS();
	UFUNCTION(BlueprintCallable) void EndADS();
	float CurrentADSTime = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS") float ADSTime = 0.2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS") float TargetADSXOffset = 41;
	float CurrentADSXOffset = TargetADSXOffset;
	UFUNCTION(BlueprintCallable) void SetSight(USceneComponent* SightToSet, float Offset, FRotator SightRotation);
	// 准星位置修正
	void ADSCorrection(FVector TotalOffset, FRotator TotalRotationOffset, float DeltaTime);
	FVector TargetSightOffset;
	FVector CurrentSightOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS") float SightOffsetInterpolationRate = 5.f;
	float ADSAlpha = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS") float ADSInterpolationRate = 5.f; //ADS插值速率
};
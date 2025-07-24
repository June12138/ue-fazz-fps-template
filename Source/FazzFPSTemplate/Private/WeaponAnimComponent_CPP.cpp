// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponAnimComponent_CPP.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// Sets default values for this component's properties
UWeaponAnimComponent_CPP::UWeaponAnimComponent_CPP()
{
	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = true;
	SetComponentTickEnabled(true);
}


// Called when the game starts
void UWeaponAnimComponent_CPP::BeginPlay()
{
	Super::BeginPlay();
	// ...
}

void UWeaponAnimComponent_CPP::Init(USceneComponent* WeaponRootToSet, UCameraComponent* CameraRootToSet)
{
	WeaponRoot = WeaponRootToSet;
	CameraRoot = CameraRootToSet;
	CurrentBobResult = FVector::ZeroVector;
	if (WeaponRoot) {
		StartLocation = WeaponRoot->GetRelativeLocation();
		StartRotation = FVector(WeaponRoot->GetRelativeRotation().Pitch, WeaponRoot->GetRelativeRotation().Yaw, WeaponRoot->GetRelativeRotation().Roll);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("WeaponRoot is null"));
	}
}

void UWeaponAnimComponent_CPP::UpdateBob()
{
	float MoveSize = InputVector.Size();
	float TargetFrequencyMultiplier;
	float TargetLongitudeZ;
	float TargetLongitudeY;
	float TargetNoise;
	// 根据移动状态设置参数
	if (MoveSize > 0.1f)
	{
		TargetFrequencyMultiplier = BobFrequencyMultiplierWalk * MoveSize;
		TargetLongitudeZ = BobLongitudeWalkZ * MoveSize;
		TargetLongitudeY = BobLongitudeWalkY * MoveSize;
		TargetNoise = BobNoiseWalk * MoveSize;
	}
	else
	{
		TargetFrequencyMultiplier = BobFrequencyMultiplierIdle;
		TargetLongitudeZ = BobLongitudeIdleZ;
		TargetLongitudeY = BobLongitudeIdleY;
		TargetNoise = BobNoiseIdle;
	}

	// 计算目标 Bob 位移
	FVector TargetBobResult;
	float Z = FMath::Sin(ElapsedTime * TargetFrequencyMultiplier * 2 + PI * 0.25) * TargetLongitudeZ + FMath::PerlinNoise1D(ElapsedTime) * TargetNoise;
	float Y = FMath::Sin(ElapsedTime * TargetFrequencyMultiplier) * TargetLongitudeY + FMath::PerlinNoise1D(ElapsedTime) * TargetNoise;
	if (Y <= 0.f) Y *= 0.25f;
	TargetBobResult = FVector(0.f, Y, Z);

	// 将 TargetBobResult 存储为目标值，插值在 TickComponent 中处理
	BobResult = TargetBobResult;
}

void UWeaponAnimComponent_CPP::SetInputVector(FVector Vector)
{
	InputVector = Vector;
}

// Called every frame
void UWeaponAnimComponent_CPP::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ElapsedTime += DeltaTime;

	FVector RecoilResult = FVector(0.f, 0.f, 0.f);
	FVector RecoilRotationResult = FVector(0.f, 0.f, 0.f);

	// 后坐力处理
	if (IsPlayingRecoilAnim && RecoilCurve)
	{
		CurrentRecoilTime = FMath::Clamp(CurrentRecoilTime + DeltaTime, 0.f, RecoilAnimTime);
		float alpha = RecoilCurve->GetFloatValue(CurrentRecoilTime / RecoilAnimTime);
		RecoilResult = FMath::Lerp(FVector(0.f, 0.f, 0.f), CurrentRecoilOffset, alpha);
		RecoilRotationResult = FMath::Lerp(FVector(0.f, 0.f, 0.f), CurrentRecoilRotationOffset, alpha);
		if (CurrentRecoilTime == RecoilAnimTime)
		{
			IsPlayingRecoilAnim = false;
			CurrentRecoilTime = 0.f;
		}
	}

	// 武器摇晃处理
	UpdateBob();
	CurrentBobResult = FMath::VInterpTo(CurrentBobResult, BobResult, DeltaTime, BobInterpolationRate);

	// 合并结果
	Result = StartLocation + RecoilResult + CurrentBobResult;
	RotationResult = StartRotation + RecoilRotationResult;

	// 应用到武器
	if (WeaponRoot)
	{
		WeaponRoot->SetRelativeLocation(Result);
		WeaponRoot->SetRelativeRotation(FRotator(RotationResult.X, RotationResult.Y, RotationResult.Z));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponRoot nullptr"));
	}
}

void UWeaponAnimComponent_CPP::UpdateRecoilEnd()
{
	CurrentRecoilOffset = JitterVector(RecoilOffset, RecoilOffsetJitter);
	CurrentRecoilRotationOffset = JitterVector(RecoilRotationOffset, RecoilRotationOffsetJitter);
}

void UWeaponAnimComponent_CPP::StartRecoilAnim()
{
	UpdateRecoilEnd();
	IsPlayingRecoilAnim = true;
}

FVector UWeaponAnimComponent_CPP::JitterVector(FVector Input, FVector Jitter) {
	float X = FMath::RandRange(Input.X - Jitter.X / 2, Input.X + Jitter.X / 2);
	float Y = FMath::RandRange(Input.Y - Jitter.Y / 2, Input.Y + Jitter.Y / 2);
	float Z = FMath::RandRange(Input.Z - Jitter.Z / 2, Input.Z + Jitter.Z / 2);
	return FVector(X, Y, Z);
}
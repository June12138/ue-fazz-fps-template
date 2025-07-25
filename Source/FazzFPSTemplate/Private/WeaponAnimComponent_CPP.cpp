// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponAnimComponent_CPP.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"

// Sets default values for this component's properties
UWeaponAnimComponent_CPP::UWeaponAnimComponent_CPP()
{
	PrimaryComponentTick.bCanEverTick = true;
	//bAutoActivate = true;
	SetComponentTickEnabled(true);
}


// Called when the game starts
void UWeaponAnimComponent_CPP::BeginPlay()
{
	Super::BeginPlay();
	// ...
}

void UWeaponAnimComponent_CPP::Init(USceneComponent* WeaponRootToSet, USceneComponent* SightToSet, UCameraComponent* CameraRootToSet)
{
	WeaponRoot = WeaponRootToSet;
	Sight = SightToSet;
	CameraRoot = CameraRootToSet;
	CurrentBobResult = FVector::ZeroVector;
	if (WeaponRoot && CameraRoot && Sight) {
		DefaultLocation = WeaponRoot->GetRelativeLocation();
		DefaultRotation = WeaponRoot->GetRelativeRotation();
		//设置ADS基准位置
		SightRelativeTransform = UKismetMathLibrary::MakeRelativeTransform(Sight->GetComponentTransform(), CameraRoot->GetComponentTransform());
		AimLocation = DefaultLocation - SightRelativeTransform.GetLocation() + FVector(ADSXOffset, 0.f, 0.f);
		Sight_RootOffset = UKismetMathLibrary::MakeRelativeTransform(Sight->GetComponentTransform(), WeaponRoot->GetComponentTransform()).GetLocation();
		//UE_LOG(LogTemp, Warning, TEXT("%f %f %f"), SightRelativeTransform.GetLocation().X, SightRelativeTransform.GetLocation().Y, SightRelativeTransform.GetLocation().Z);
		//UE_LOG(LogTemp, Warning, TEXT("%f %f %f"), SightRelativeTransform.GetRotation().Rotator().Pitch, SightRelativeTransform.GetRotation().Rotator().Yaw, SightRelativeTransform.GetRotation().Rotator().Roll);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("WeaponAnimComponent Init failed"));
	}
}

void UWeaponAnimComponent_CPP::UpdateBob()
{
	// 根据移动状态设置参数
	float multiplier = 1.f;
	float MoveSize = InputVector.Size();
	if (MoveSize > 0.1f)
	{
		multiplier = MoveSize;
		CurrentBob = &WalkBob;
	}
	else
	{
		CurrentBob = &IdleBob;
	}
	// 计算目标 Bob 位移
	float HorizontalMultiplier = FMath::Sin(ElapsedTime * CurrentBob->BobFrequencyMultiplier * 2 + PI * 0.25);
	float VerticalMultiplier = FMath::Sin(ElapsedTime * CurrentBob->BobFrequencyMultiplier);
	float Noise = FMath::PerlinNoise1D(ElapsedTime) * CurrentBob->BobNoise;
	if (VerticalMultiplier <= 0.f) VerticalMultiplier *= 0.25f;
	float Z = HorizontalMultiplier * CurrentBob->BobLongitudeZ + Noise;
	float Y = VerticalMultiplier * CurrentBob->BobLongitudeY + Noise;
	float Pitch = VerticalMultiplier * CurrentBob->BobPitch + Noise;
    float Yaw = HorizontalMultiplier * CurrentBob->BobYaw + Noise;
	BobResult = FVector(0.f, Y, Z);
	BobResultRot = FRotator(Pitch, Yaw, 0.f);
}

void UWeaponAnimComponent_CPP::SetInput(FVector Vector, FRotator Rotator)
{
	InputVector = Vector;
	InputRotator = Rotator;
}

// Called every frame
void UWeaponAnimComponent_CPP::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!ShouldPlayAnimation) return;
	ElapsedTime += DeltaTime;
	//ADS处理
	if (PlayingADSAnimation && ADSCurve) {
		if (ToADS) {
			CurrentADSTime = FMath::Clamp(CurrentADSTime + DeltaTime, 0.f, ADSTime);
			if (CurrentADSTime == ADSTime)
			{
				PlayingADSAnimation = false;
				IsAiming = true;
			}
		}
		else {
			CurrentADSTime = FMath::Clamp(CurrentADSTime - DeltaTime, 0.f, ADSTime);
			if (CurrentADSTime == 0.f)
			{
				PlayingADSAnimation = false;
			}
		}
		FVector Offset = -SightRelativeTransform.GetLocation() + FVector(ADSXOffset, 0.f, 0.f);
		float alpha = ADSCurve->GetFloatValue(CurrentADSTime / ADSTime);
		CurrentADSOffset = FMath::Lerp(FVector(0.f, 0.f, 0.f), Offset, alpha);
	}
	//武器后坐处理
	FVector RecoilResult = FVector::ZeroVector;
	FRotator RecoilRotationResult = FRotator::ZeroRotator;
	if (IsPlayingRecoilAnim && RecoilCurve)
	{
		CurrentRecoilTime = FMath::Clamp(CurrentRecoilTime + DeltaTime, 0.f, RecoilAnimTime);
		float alpha = RecoilCurve->GetFloatValue(CurrentRecoilTime / RecoilAnimTime);
		RecoilResult = FMath::Lerp(FVector(0.f, 0.f, 0.f), CurrentRecoilOffset, alpha);
		FVector RotationVector = FMath::Lerp(FVector(0.f, 0.f, 0.f), CurrentRecoilRotationOffset, alpha);
		RecoilRotationResult = FRotator(RotationVector.X, RotationVector.Y, RotationVector.Z);
		if (CurrentRecoilTime == RecoilAnimTime)
		{
			IsPlayingRecoilAnim = false;
			CurrentRecoilTime = 0.f;
		}
	}
	//武器摇晃处理
	UpdateBob();
	CurrentBobResult = FMath::VInterpTo(CurrentBobResult, BobResult, DeltaTime, BobInterpolationRate);
	CurrentBobResultRot = FMath::RInterpTo(CurrentBobResultRot, BobResultRot, DeltaTime, BobRotationInterpolationRate);
	//武器Sway处理
	UpdateSway();
	CurrentSway = FMath::RInterpTo(CurrentSway, TargetSway, DeltaTime, SwayInterpolationRate);
	// 合并结果
	FVector TotalOffset = RecoilResult + CurrentBobResult + CurrentADSOffset;
	FRotator TotalRotationOffset = FRotator(RecoilRotationResult.Quaternion() * CurrentSway.Quaternion() * CurrentBobResultRot.Quaternion());
	Result = *TargetBaseLocation + TotalOffset + CorrectSightLocation(TotalOffset, TotalRotationOffset, DeltaTime);
	RotationResult = FRotator(TargetBaseRotation->Quaternion() * RecoilRotationResult.Quaternion() * CurrentSway.Quaternion() * CurrentBobResultRot.Quaternion());
	if (WeaponRoot)
	{
		WeaponRoot->SetRelativeLocation(Result);
		WeaponRoot->SetRelativeRotation(RotationResult);
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

void UWeaponAnimComponent_CPP::UpdateSway() {
	float Yaw = FMath::Clamp(InputRotator.Yaw * SwayYawMultiplier * -1, -SwayYawMax/2, SwayYawMax/2);
	float Pitch = FMath::Clamp(InputRotator.Pitch * SwayPitchMultiplier, -SwayPitchMax/2, SwayPitchMax/2);
	TargetSway = FRotator(Pitch, Yaw, 0.f);
}

void UWeaponAnimComponent_CPP::StartADS()
{
	ToADS = true;
	PlayingADSAnimation = true;
}

void UWeaponAnimComponent_CPP::EndADS()
{
	ToADS = false;
	IsAiming = false;
	PlayingADSAnimation = true;
}

FVector UWeaponAnimComponent_CPP::CorrectSightLocation(FVector TotalOffset, FRotator TotalRotationOffset, float DeltaTime)
{
	FVector SightCorrection = FVector::ZeroVector;
	if (IsAiming) {
		FVector PredictedSightLocation = *TargetBaseLocation + TotalOffset + TotalRotationOffset.RotateVector(Sight_RootOffset);
		FVector PredictedDeviation = PredictedSightLocation - FVector(ADSXOffset, 0.f, 0.f);
		SightCorrection = -1 * PredictedDeviation;
		SightCorrectionAlpha = FMath::Clamp(SightCorrectionAlpha + DeltaTime * SightCorrectionSpeed_ToADS, 0.f, 1.f);
	}
	else {
		SightCorrectionAlpha = FMath::Clamp(SightCorrectionAlpha - DeltaTime * SightCorrectionSpeed_FromADS, 0.f, 1.f);
	}
	return SightCorrection * SightCorrectionAlpha;
}

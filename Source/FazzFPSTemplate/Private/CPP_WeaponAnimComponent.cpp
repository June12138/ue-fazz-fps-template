// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_WeaponAnimComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"

// Sets default values for this component's properties
UCPP_WeaponAnimComponent::UCPP_WeaponAnimComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	//bAutoActivate = true;
	SetComponentTickEnabled(true);
}


// Called when the game starts
void UCPP_WeaponAnimComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
}

void UCPP_WeaponAnimComponent::Init(USceneComponent* WeaponRootToSet, USceneComponent* SightToSet, UCameraComponent* CameraRootToSet)
{
	WeaponRoot = WeaponRootToSet;
	Sight = SightToSet;
	CameraRoot = CameraRootToSet;
	CurrentBobResult = FVector::ZeroVector;
	if (WeaponRoot && CameraRoot && Sight) {
		DefaultBaseLocation = WeaponRoot->GetRelativeLocation();
		CurrentBaseLocation = DefaultBaseLocation;
		DefaultBaseRotation = WeaponRoot->GetRelativeRotation();
		CurrentBaseRotation = DefaultBaseRotation;
		//设置ADS基准位置
		SightRelativeTransform = UKismetMathLibrary::MakeRelativeTransform(Sight->GetComponentTransform(), CameraRoot->GetComponentTransform());
		ADSBaseLocation = DefaultBaseLocation - SightRelativeTransform.GetLocation() + FVector(ADSXOffset, 0.f, 0.f);
		Sight_RootOffset = UKismetMathLibrary::MakeRelativeTransform(Sight->GetComponentTransform(), WeaponRoot->GetComponentTransform()).GetLocation();
		//UE_LOG(LogTemp, Warning, TEXT("%f %f %f"), SightRelativeTransform.GetLocation().X, SightRelativeTransform.GetLocation().Y, SightRelativeTransform.GetLocation().Z);
		//UE_LOG(LogTemp, Warning, TEXT("%f %f %f"), SightRelativeTransform.GetRotation().Rotator().Pitch, SightRelativeTransform.GetRotation().Rotator().Yaw, SightRelativeTransform.GetRotation().Rotator().Roll);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("WeaponAnimComponent Init failed"));
	}
}
void UCPP_WeaponAnimComponent::SetInput(FVector Vector, FRotator Rotator)
{
	InputVector = Vector;
	InputRotator = Rotator;
}

void UCPP_WeaponAnimComponent::StartSprint()
{
	CurrentStance = EStanceState::Sprint;
}

void UCPP_WeaponAnimComponent::EndSprint()
{
	CurrentStance = EStanceState::Default;
}

void UCPP_WeaponAnimComponent::StartCrouch()
{
	CurrentStance = EStanceState::Crouch;
}

void UCPP_WeaponAnimComponent::EndCrouch()
{
	CurrentStance = EStanceState::Default;
}

// Called every frame
void UCPP_WeaponAnimComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!ShouldPlayAnimation) return;
	UpdateSettings();
	ElapsedTime += DeltaTime;
	// 更新基准位置和旋转
	CurrentBaseLocation = FMath::VInterpTo(CurrentBaseLocation, *TargetBaseLocation, DeltaTime, BaseLocationInterpolationRate);
	CurrentBaseRotation = FMath::RInterpTo(CurrentBaseRotation, *TargetBaseRotation, DeltaTime, BaseRotationInterpolationRate);
	//武器后坐处理
	FRotator RecoilRotationResult = FRotator::ZeroRotator;
	if (IsPlayingRecoilAnim && RecoilCurve)
	{
		CurrentRecoilTime = FMath::Clamp(CurrentRecoilTime + DeltaTime, 0.f, RecoilAnimTime);
		float alpha = RecoilCurve->GetFloatValue(CurrentRecoilTime / RecoilAnimTime);
		CurrentRecoilOffset = FMath::Lerp(FVector(0.f, 0.f, 0.f), RecoilTargetOffset, alpha);
		FVector RotationVector = FMath::Lerp(FVector(0.f, 0.f, 0.f), RecoilRotationTargetOffset, alpha);
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
	FVector TotalOffset = CurrentRecoilOffset + CurrentBobResult;
	FRotator TotalRotationOffset = FRotator(RecoilRotationResult.Quaternion() * CurrentSway.Quaternion() * CurrentBobResultRot.Quaternion());
	//ADS处理
	ADSCorrection(TotalOffset, TotalRotationOffset, DeltaTime);
	CurrentADSCorrection = FMath::VInterpTo(CurrentADSCorrection, TargetADSCorrection, DeltaTime, ADSInterpolationRate);
	Result = CurrentBaseLocation + TotalOffset;
	RotationResult = FRotator(CurrentBaseRotation.Quaternion() * RecoilRotationResult.Quaternion() * CurrentSway.Quaternion() * CurrentBobResultRot.Quaternion());
	if (WeaponRoot)
	{
		WeaponRoot->SetRelativeLocation(Result + CurrentADSCorrection);
		WeaponRoot->SetRelativeRotation(RotationResult);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponRoot nullptr"));
	}
}

void UCPP_WeaponAnimComponent::UpdateRecoilEnd()
{
	RecoilTargetOffset = JitterVector(CurrentRecoilStruct->RecoilOffset, CurrentRecoilStruct->RecoilOffsetJitter);
	RecoilRotationTargetOffset = JitterVector(CurrentRecoilStruct->RecoilRotationOffset, CurrentRecoilStruct->RecoilRotationOffsetJitter);
}

void UCPP_WeaponAnimComponent::StartRecoilAnim()
{
	UpdateRecoilEnd();
	CurrentRecoilTime = 0.f;
	IsPlayingRecoilAnim = true;
}

FVector UCPP_WeaponAnimComponent::JitterVector(FVector Input, FVector Jitter) {
	float X = FMath::RandRange(Input.X - Jitter.X / 2, Input.X + Jitter.X / 2);
	float Y = FMath::RandRange(Input.Y - Jitter.Y / 2, Input.Y + Jitter.Y / 2);
	float Z = FMath::RandRange(Input.Z - Jitter.Z / 2, Input.Z + Jitter.Z / 2);
	return FVector(X, Y, Z);
}

void UCPP_WeaponAnimComponent::StartADS()
{
	ToADS = true;
	PlayingADSAnimation = true;
}

void UCPP_WeaponAnimComponent::EndADS(bool UseCurve)
{
	ToADS = false;
	IsAiming = false;
	PlayingADSAnimation = true;
	if (!UseCurve) { CurrentADSTime = 0.f; }
}
void UCPP_WeaponAnimComponent::UpdateSway() {
	float Yaw = FMath::Clamp(InputRotator.Yaw * CurrentSwayStruct->SwayYawMultiplier * -1, -CurrentSwayStruct->SwayYawMax/2, CurrentSwayStruct->SwayYawMax/2);
	float Pitch = FMath::Clamp(InputRotator.Pitch * CurrentSwayStruct->SwayPitchMultiplier, -CurrentSwayStruct->SwayPitchMax/2, CurrentSwayStruct->SwayPitchMax/2);
	TargetSway = FRotator(Pitch, Yaw, 0.f);
}
void UCPP_WeaponAnimComponent::UpdateBob()
{
	// 根据移动状态设置参数
	float multiplier = 1.f;
	float MoveSize = InputVector.Size();
	if (MoveSize > 0.01f)
	{
		multiplier = MoveSize;
	}
	// 计算目标 Bob 位移
	float HorizontalMultiplier = FMath::Sin(ElapsedTime * CurrentBob->BobFrequencyMultiplier * 2 + PI * 0.25) * multiplier;
	float VerticalMultiplier = FMath::Sin(ElapsedTime * CurrentBob->BobFrequencyMultiplier) * multiplier;
	float Noise = FMath::PerlinNoise1D(ElapsedTime) * CurrentBob->BobNoise * multiplier;
	if (VerticalMultiplier <= 0.f) VerticalMultiplier *= 0.25f;
	float Z = HorizontalMultiplier * CurrentBob->BobLongitudeZ + Noise;
	float Y = VerticalMultiplier * CurrentBob->BobLongitudeY + Noise;
    float Yaw = FMath::Sin(ElapsedTime * CurrentBob->BobFrequencyMultiplier + PI * 0.25) * CurrentBob->BobYaw + Noise;
	float Pitch = FMath::Abs(FMath::Sin(ElapsedTime * CurrentBob->BobFrequencyMultiplier)) * CurrentBob->BobPitch + Noise;
	BobResult = FVector(0.f, Y, Z);
	BobResultRot = FRotator(Pitch, Yaw, 0.f);
}
void UCPP_WeaponAnimComponent::ADSCorrection(FVector TotalOffset, FRotator TotalRotationOffset, float DeltaTime)
{
	// 根据曲线和时间计算ADS动画的插值
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
		ADSAlpha = ADSCurve->GetFloatValue(CurrentADSTime / ADSTime);
	}
	// 根据TotalOffset和TotalRotationOffset预测结算后的准星的相对位置
	FVector PredictedSightLocation = CurrentBaseLocation + TotalOffset + CurrentBaseRotation.RotateVector(TotalRotationOffset.RotateVector(Sight_RootOffset));
	FVector PredictedDeviation = PredictedSightLocation - FVector(ADSXOffset, 0.f, 0.f) - CurrentRecoilOffset;
	TargetADSCorrection = -1 * PredictedDeviation * ADSAlpha;
}
void UCPP_WeaponAnimComponent::UpdateSettings()
{
	float MoveSize = InputVector.Size();
	switch (CurrentStance)
	{
		case EStanceState::Default:
			TargetBaseRotation = &DefaultBaseRotation;
			TargetBaseLocation = &DefaultBaseLocation;
			CurrentRecoilStruct = &DefaultRecoilStruct;
			if (MoveSize > 0.1f) {
				CurrentBob = &WalkBob;
			}	else {
				CurrentBob = &IdleBob;
			}
			break;
		case EStanceState::Sprint:
			CurrentBob = &RunBob;
			TargetBaseLocation = &SprintBaseLocation;
			TargetBaseRotation = &SprintBaseRotation;
			break;
		case EStanceState::Crouch:
			TargetBaseLocation = &CrouchBaseLocation;
			TargetBaseRotation = &CrouchBaseRotation;
			if (MoveSize > 0.1f) {
				CurrentBob = &WalkBob;
			}	else {
				CurrentBob = &IdleBob;
			}
			break;
	}
	//ADS
	if (IsAiming || PlayingADSAnimation) {
		CurrentSwayStruct = &ADSSway;
		TargetBaseRotation = &ADSBaseRotation;
		TargetBaseLocation = &DefaultBaseLocation;
		CurrentRecoilStruct = &ADSRecoilStruct;
		if (MoveSize > 0.1f){
			CurrentBob = &WalkBobADS;
		}else{
			CurrentBob = &IdleBobADS;
		}
	}
}
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
		CurrentBaseLocation = DefaultLocation;
		DefaultRotation = WeaponRoot->GetRelativeRotation();
		CurrentBaseRotation = DefaultRotation;
		//����ADS��׼λ��
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
	// ���»�׼λ�ú���ת
	CurrentBaseLocation = FMath::VInterpTo(CurrentBaseLocation, *TargetBaseLocation, DeltaTime, BaseLocationInterpolationRate);
	CurrentBaseRotation = FMath::RInterpTo(CurrentBaseRotation, *TargetBaseRotation, DeltaTime, BaseRotationInterpolationRate);
	//������������
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
	//����ҡ�δ���
	UpdateBob();
	CurrentBobResult = FMath::VInterpTo(CurrentBobResult, BobResult, DeltaTime, BobInterpolationRate);
	CurrentBobResultRot = FMath::RInterpTo(CurrentBobResultRot, BobResultRot, DeltaTime, BobRotationInterpolationRate);
	//����Sway����
	UpdateSway();
	CurrentSway = FMath::RInterpTo(CurrentSway, TargetSway, DeltaTime, SwayInterpolationRate);
	// �ϲ����
	FVector TotalOffset = RecoilResult + CurrentBobResult;
	FRotator TotalRotationOffset = FRotator(RecoilRotationResult.Quaternion() * CurrentSway.Quaternion() * CurrentBobResultRot.Quaternion());
	//ADS����
	ADSCorrection(&TotalOffset, TotalRotationOffset, DeltaTime);
	Result = CurrentBaseLocation + TotalOffset;
	RotationResult = FRotator(CurrentBaseRotation.Quaternion() * RecoilRotationResult.Quaternion() * CurrentSway.Quaternion() * CurrentBobResultRot.Quaternion());
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

void UWeaponAnimComponent_CPP::StartADS()
{
	ToADS = true;
	PlayingADSAnimation = true;
	TargetBaseRotation = &ADSBaseRotation;
}

void UWeaponAnimComponent_CPP::EndADS()
{
	ToADS = false;
	IsAiming = false;
	PlayingADSAnimation = true;
	TargetBaseRotation = &DefaultRotation;
}
void UWeaponAnimComponent_CPP::UpdateSway() {
	float Yaw = FMath::Clamp(InputRotator.Yaw * SwayYawMultiplier * -1, -SwayYawMax/2, SwayYawMax/2);
	float Pitch = FMath::Clamp(InputRotator.Pitch * SwayPitchMultiplier, -SwayPitchMax/2, SwayPitchMax/2);
	TargetSway = FRotator(Pitch, Yaw, 0.f);
}
void UWeaponAnimComponent_CPP::UpdateBob()
{
	// �����ƶ�״̬���ò���
	float multiplier = 1.f;
	float MoveSize = InputVector.Size();
	if (MoveSize > 0.01f)
	{
		multiplier = MoveSize;
		CurrentBob = &WalkBob;
		if (IsAiming || PlayingADSAnimation) {
			CurrentBob = &WalkBobADS;
		}
	}else{
		CurrentBob = &IdleBob;
		if (IsAiming || PlayingADSAnimation) {
			CurrentBob = &IdleBobADS;
		}
	}
	// ����Ŀ�� Bob λ��
	float HorizontalMultiplier = FMath::Sin(ElapsedTime * CurrentBob->BobFrequencyMultiplier * 2 + PI * 0.25) * multiplier;
	float VerticalMultiplier = FMath::Sin(ElapsedTime * CurrentBob->BobFrequencyMultiplier) * multiplier;
	float Noise = FMath::PerlinNoise1D(ElapsedTime) * CurrentBob->BobNoise * multiplier;
	if (VerticalMultiplier <= 0.f) VerticalMultiplier *= 0.25f;
	float Z = HorizontalMultiplier * CurrentBob->BobLongitudeZ + Noise;
	float Y = VerticalMultiplier * CurrentBob->BobLongitudeY + Noise;
	float Pitch = VerticalMultiplier * CurrentBob->BobPitch + Noise;
    float Yaw = HorizontalMultiplier * CurrentBob->BobYaw + Noise;
	BobResult = FVector(0.f, Y, Z);
	BobResultRot = FRotator(Pitch, Yaw, 0.f);
}
void UWeaponAnimComponent_CPP::ADSCorrection(FVector* TotalOffset, FRotator TotalRotationOffset, float DeltaTime)
{
	// �������ߺ�ʱ�����ADS�����Ĳ�ֵ
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
	// ����TotalOffset��TotalRotationOffsetԤ�������׼�ǵ����λ��
	FVector PredictedSightLocation = CurrentBaseLocation + *TotalOffset + TargetBaseRotation->RotateVector(TotalRotationOffset.RotateVector(Sight_RootOffset));
	FVector PredictedDeviation = PredictedSightLocation - FVector(ADSXOffset, 0.f, 0.f);
	*TotalOffset += -1 * PredictedDeviation * ADSAlpha;
}

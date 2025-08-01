// Fill out your copyright notice in the Description page of Project Settings.

#include "CPP_WeaponAnimComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"

// Sets default values for this component's properties
UCPP_WeaponAnimComponent::UCPP_WeaponAnimComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// bAutoActivate = true;
	SetComponentTickEnabled(true);
}

// Called when the game starts
void UCPP_WeaponAnimComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
}

void UCPP_WeaponAnimComponent::Init(USceneComponent *WeaponRootToSet, USceneComponent *SightToSet, USceneComponent *CameraRootToSet)
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
		// 设置ADS基准位置
		SightRelativeTransform = UKismetMathLibrary::MakeRelativeTransform(Sight->GetComponentTransform(), CameraRoot->GetComponentTransform());
		ADSBaseLocation = DefaultBaseLocation - SightRelativeTransform.GetLocation() + FVector(ADSXOffset, 0.f, 0.f);
		Sight_RootOffset = UKismetMathLibrary::MakeRelativeTransform(Sight->GetComponentTransform(), WeaponRoot->GetComponentTransform()).GetLocation();
		CamInitialLocation = CameraRoot->GetRelativeLocation();
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("WeaponAnimComponent Init failed"));
	}
}
void UCPP_WeaponAnimComponent::SetInputVector(FVector Vector)
{
	InputVector = Vector;
	InputVector2D = FVector2D(Vector.X, Vector.Y);
}

void UCPP_WeaponAnimComponent::SetInputRotator(FRotator Rotator)
{
	InputRotator = Rotator;
}

void UCPP_WeaponAnimComponent::StartSprint()
{
	if (CurrentStance != EStanceState::Sprint){
		CurrentStance = EStanceState::Sprint;
	}
}

void UCPP_WeaponAnimComponent::EndSprint()
{
	if (CurrentStance == EStanceState::Sprint){
		CurrentStance = EStanceState::Default;
	}
}

void UCPP_WeaponAnimComponent::StartCrouch()
{
	if (CurrentStance != EStanceState::Crouch){
		CurrentStance = EStanceState::Crouch;
	}
}

void UCPP_WeaponAnimComponent::EndCrouch()
{
	if (CurrentStance == EStanceState::Crouch){
		CurrentStance = EStanceState::Default;
	}
}

// Called every frame
void UCPP_WeaponAnimComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!ShouldPlayAnimation) return;
	UpdateSettings();
	ElapsedTime += DeltaTime;
	// 更新基准位置和旋转
	CurrentBaseLocation = FMath::VInterpTo(CurrentBaseLocation, *TargetBaseLocation, DeltaTime, BaseLocationInterpolationRate);
	CurrentBaseRotation = FMath::RInterpTo(CurrentBaseRotation, *TargetBaseRotation, DeltaTime, BaseRotationInterpolationRate);
	// 侧头处理
	UpdateTilt(DeltaTime);
	//UE_LOG(LogTemp, Warning, TEXT("CurrentTiltOffset: %s, CurrentTiltRoll: %f"), *CurrentTiltOffset.ToString(), CurrentTiltRoll);
	// 武器后坐处理
	FRotator RecoilRotationResult = FRotator::ZeroRotator;
	if (IsPlayingRecoilAnim && RecoilCurve)
	{
		CurrentRecoilTime = FMath::Clamp(CurrentRecoilTime + DeltaTime, 0.f, RecoilAnimTime);
		float alpha = RecoilCurve->GetFloatValue(CurrentRecoilTime / RecoilAnimTime);
		CurrentRecoilOffset = FMath::Lerp(FVector(0.f, 0.f, 0.f), RecoilTargetOffset, alpha);
		FVector RotationVector = FMath::Lerp(FVector(0.f, 0.f, 0.f), RecoilRotationTargetOffset, alpha);
		RecoilRotationResult = FRotator(RotationVector.X, RotationVector.Y, RotationVector.Z);
		CurrentRecoilGradualOffset = FMath::VInterpTo(CurrentRecoilGradualOffset, CurrentRecoilStruct->RecoilGradualOffset, DeltaTime, CurrentRecoilStruct->RecoilGradualOffsetInterpolationRate);
		CurrentRecoilGradualRotOffset = FMath::RInterpTo(CurrentRecoilGradualRotOffset, CurrentRecoilStruct->RecoilGradualRotationOffset, DeltaTime, CurrentRecoilStruct->RecoilGradualOffsetInterpolationRate);
		if (CurrentRecoilTime == RecoilAnimTime)
		{
			IsPlayingRecoilAnim = false;
			CurrentRecoilTime = 0.f;
		}
	}else{
		if (!RecoilCurve){
			UE_LOG(LogTemp, Error, TEXT("RecoilCurve nullptr"));
		}
		CurrentRecoilGradualOffset = FMath::VInterpTo(CurrentRecoilGradualOffset, FVector::ZeroVector, DeltaTime, CurrentRecoilStruct->RecoilGradualOffsetRecoverRate);
		CurrentRecoilGradualRotOffset = FMath::RInterpTo(CurrentRecoilGradualRotOffset, FRotator::ZeroRotator, DeltaTime, CurrentRecoilStruct->RecoilGradualOffsetRecoverRate);
	}
	// 武器摇晃处理
	UpdateBob();
	CurrentBobResult = FMath::VInterpTo(CurrentBobResult, BobResult, DeltaTime, BobInterpolationRate);
	CurrentBobResultRot = FMath::RInterpTo(CurrentBobResultRot, BobResultRot, DeltaTime, BobRotationInterpolationRate);
	// 武器Sway处理
	UpdateSway();
	CurrentSway = FMath::RInterpTo(CurrentSway, TargetSway, DeltaTime, SwayInterpolationRate);
	// MovementOffset处理
	UpdateMovementOffset();
	CurrentMovementOffset = FMath::VInterpTo(CurrentMovementOffset, TargetMovementOffset, DeltaTime, MovementOffsetInterpolationRate);
	CurrentMovementRotationOffset = FMath::FInterpTo(CurrentMovementRotationOffset, TargetMovementRotationOffset, DeltaTime, MovementOffsetInterpolationRate);
	// 跳跃处理
	UpdateJump(DeltaTime);
	// 合并结果
	FVector TotalOffset = CurrentRecoilOffset + CurrentBobResult + CurrentMovementOffset + FVector(0.f, 0.f, CurrentJumpOffset) + CurrentRecoilGradualOffset;
	FRotator TotalRotationOffset = RecoilRotationResult + CurrentSway + CurrentBobResultRot + CurrentRecoilGradualRotOffset + FRotator(0.f, 0.f, CurrentMovementRotationOffset);
	// ADS处理
	ADSCorrection(TotalOffset, TotalRotationOffset, DeltaTime);
	CurrentADSCorrection = FMath::VInterpTo(CurrentADSCorrection, TargetADSCorrection, DeltaTime, ADSInterpolationRate);
	Result = CurrentBaseLocation + TotalOffset + CurrentADSCorrection;
	RotationResult = CurrentBaseRotation + TotalRotationOffset;
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
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn)
	{
	    APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	    if (PlayerController)
	    {
	        if (PlayerController->PlayerCameraManager && CurrentRecoilStruct->RecoilCameraShakeClass)
	        {
	            PlayerController->PlayerCameraManager->StartCameraShake(CurrentRecoilStruct->RecoilCameraShakeClass, 1.f);
				//UE_LOG(LogTemp, Warning, TEXT("Executing Cam Shake"));
	        }
	    }
	}
}

FVector UCPP_WeaponAnimComponent::JitterVector(FVector Input, FVector Jitter)
{
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
void UCPP_WeaponAnimComponent::UpdateSway()
{
	float Yaw = FMath::Clamp(InputRotator.Yaw * CurrentSwayStruct->SwayYawMultiplier * -1, -CurrentSwayStruct->SwayYawMax / 2, CurrentSwayStruct->SwayYawMax / 2);
	float Pitch = FMath::Clamp(InputRotator.Pitch * CurrentSwayStruct->SwayPitchMultiplier, -CurrentSwayStruct->SwayPitchMax / 2, CurrentSwayStruct->SwayPitchMax / 2);
	TargetSway = FRotator(Pitch, Yaw, 0.f);
}
void UCPP_WeaponAnimComponent::UpdateBob()
{
	// 根据移动状态设置参数
	float multiplier = 1.f;
	MoveSize = InputVector2D.Size();
	if (MoveSize > 0.01f)
	{
		multiplier = MoveSize;
	}
	if (CurrentStance == EStanceState::Crouch) {
		multiplier *= CrouchMultiplier;
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
	FVector PredictedDeviation = PredictedSightLocation - FVector(ADSXOffset, 0.f, 0.f) - CurrentRecoilOffset - CurrentRecoilGradualOffset;
	TargetADSCorrection = -1 * PredictedDeviation * ADSAlpha;
}
void UCPP_WeaponAnimComponent::UpdateMovementOffset()
{
	if (IsAiming || PlayingADSAnimation) {
		TargetMovementOffset = FVector::ZeroVector;
	}else{
		float X = InputVector.Y * MovementOffsetMax.X * -1;
		float Y = InputVector.X * MovementOffsetMax.Y * -1;
		float Z = InputVector.Z * MovementOffsetMax.Z * -1;
		TargetMovementOffset = FVector(X, Y, Z);
	}
	TargetMovementRotationOffset = InputVector.X * MovementRotationOffsetMax;
}
void UCPP_WeaponAnimComponent::UpdateSettings()
{
	MoveSize = InputVector2D.Size();
	CurrentSwayStruct = &DefaultSway;
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
	// ADS
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
void UCPP_WeaponAnimComponent::StartJump()
{
	if (CurrentJumpState == EJumpState::Default) {
		CurrentJumpState = EJumpState::Start;
	}
}
void UCPP_WeaponAnimComponent::MidAir()
{
	if (CurrentJumpState != EJumpState::Start) {
		CurrentJumpState = EJumpState::MidAir;
	}
}
void UCPP_WeaponAnimComponent::EndJump()
{
    if (CurrentJumpState == EJumpState::MidAir || CurrentJumpState == EJumpState::Start) {
		CurrentJumpState = EJumpState::Land;
	}
}
void UCPP_WeaponAnimComponent::UpdateJump(float DeltaTime){
	UpdateJumpState();
	//CurrentJumpOffset = FMath::FInterpConstantTo(CurrentJumpOffset, *TargetJumpOffset, DeltaTime, *CurrentJumpInterpolationRate);
	CurrentJumpOffset = FMath::FInterpTo(CurrentJumpOffset, *TargetJumpOffset, DeltaTime, *CurrentJumpInterpolationRate);
	//UE_LOG(LogTemp, Warning, TEXT("CurrentJumpOffset: %f/%f: %f"), CurrentJumpOffset, *TargetJumpOffset, *CurrentJumpInterpolationRate);
}

void UCPP_WeaponAnimComponent::UpdateJumpState()
{
		switch (CurrentJumpState){
			case EJumpState::Default:
				CurrentJumpInterpolationRate = &JumpOffsetInterpolationRateUp;
				TargetJumpOffset = &DefaultJumpOffset;
				break;
			case EJumpState::Start:
				TargetJumpOffset = &JumpOffset;
				CurrentJumpInterpolationRate = &JumpOffsetInterpolationRateUp;
				if (FMath::Abs(CurrentJumpOffset - *TargetJumpOffset) < JumpTransitionTolerance) {
					CurrentJumpState = EJumpState::MidAir;
				}
				break;
			case EJumpState::MidAir:
				TargetJumpOffset = &MidAirOffset;
				CurrentJumpInterpolationRate = &JumpOffsetInterpolationRateUp;
				break;
			case EJumpState::Land:
				TargetJumpOffset = &LandOffset;
				CurrentJumpInterpolationRate = &JumpOffsetInterpolationRateDown;
				if (FMath::Abs(CurrentJumpOffset - *TargetJumpOffset) < JumpTransitionTolerance) {
					CurrentJumpState = EJumpState::Default;
		}
	}
}
void UCPP_WeaponAnimComponent::SetTilt(int Direction){
	if (Direction == TiltDirection){
		TiltDirection = 0;
	}else{
		TiltDirection = Direction;
	}
}
void UCPP_WeaponAnimComponent::UpdateTilt(float DeltaTime){
    	// 侧头处理
	float Alpha = 0.f;
	Alpha = FMath::Clamp(DeltaTime * TiltInterpolationRate, 0.f, 1.f);
	Alpha = 1 - FMath::Square(1 - Alpha); // 二次缓出
	switch (TiltDirection)
	{
		case 0:
			CurrentTiltOffset = FMath::Lerp(CurrentTiltOffset, FVector::ZeroVector, Alpha);
			CurrentTiltRoll = FMath::Lerp(CurrentTiltRoll, 0.f, Alpha);
			break;
		case 1:
			CurrentTiltOffset = FMath::Lerp(CurrentTiltOffset, TiltOffsetRight, Alpha);
			CurrentTiltRoll = FMath::Lerp(CurrentTiltRoll, TiltRoll, Alpha);
			break;
		case -1:
		CurrentTiltOffset = FMath::Lerp(CurrentTiltOffset, TiltOffsetLeft, Alpha);
			CurrentTiltRoll = FMath::Lerp(CurrentTiltRoll, -TiltRoll, Alpha);
			break;
	}
	if (CameraRoot){
		CameraRoot->SetRelativeLocation(CamInitialLocation + CurrentTiltOffset);
		CameraRoot->SetRelativeRotation(FRotator(0.f, 0.f, CurrentTiltRoll));
	}
}
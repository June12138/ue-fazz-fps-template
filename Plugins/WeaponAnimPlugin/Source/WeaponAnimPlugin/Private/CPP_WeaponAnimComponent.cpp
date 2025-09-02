// Fill out your copyright notice in the Description page of Project Settings.

#include "CPP_WeaponAnimComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"

// Sets default values for this component's properties
UCPP_WeaponAnimComponent::UCPP_WeaponAnimComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	//Base模块默认值
	BaseStates.Add("IdleBase", FTransform(FRotator(0.f, 0.f, 0.f), FVector(0.f, 0.f, 0.f), FVector(1.f, 1.f, 1.f)));
	BaseStates.Add("SprintBase", FTransform(FRotator(-19,-35,-24), FVector(25, -1, -13), FVector(1.f, 1.f, 1.f)));
	BaseStates.Add("CrouchBase", FTransform(FRotator(-2, -14, 3), FVector(25, 9, -8), FVector(1.f, 1.f, 1.f)));
	//Recoil模块默认值
	RecoilStates.Add("DefaultRecoil", FWeaponRecoilStruct{
		FVector(-1.f, 0.f, 0.f), //后座终止位置偏移
		FVector(1.f,1.f,0.5), //后座随机偏移
		FVector(1.f,0.f,0.f), //后座终止旋转偏移
		FVector(1.f,2.f,1.f), //后座随机旋转偏移
		FVector(-3.f,0.f,3.f), //后座旋转随机偏移 
		FRotator(6,0.f,0.f), //后座旋转渐进偏移 
		2.f, //后座旋转随机偏移插值速率
		5.f
	});
	RecoilStates.Add("ADSRecoil", FWeaponRecoilStruct{
		FVector(-0.25, 0.f, 0.3), //后座终止位置偏移
		FVector(0.1,0.05,0.f), //后座随机偏移
		FVector(0.3,0.0,0.0), //后座终止旋转偏移
		FVector(0.2,0.2,5.f), //后座随机旋转偏移
		FVector(0.f,0.f,3.f),//后座旋转随机偏移 
		FRotator(4.f,0.f,0.f), //后座旋转渐进偏移 
		2.f, //后座旋转随机偏移插值速率
		5.f
	});
	//Bob模块默认值
	BobStates.Add("IdleBob", FWeaponBobStruct{0.75, 1.f, 0.f, 3, 0, 1, 0.5});
	BobStates.Add("WalkBob", FWeaponBobStruct{4, 3, 3, 3, 3, 1, 0.7});
	BobStates.Add("RunBob", FWeaponBobStruct{8, 3, 3, 3, 3, 1, 0.7});
	BobStates.Add("IdleBobADS", FWeaponBobStruct{1, 0.f, 0.f, 0.05, 0, 1, 0.15});
	BobStates.Add("WalkBobADS", FWeaponBobStruct{4, 0.f, 0.f, 0.1, 0.2, 1, 0.15});
	//Sway模块默认值
	SwayStates.Add("DefaultSway",FWeaponSwayStruct{5, 15, 5, 15});
	SwayStates.Add("ADSSway", FWeaponSwayStruct{3, 0.3, 3, 0.3});
}

// Called when the game starts
void UCPP_WeaponAnimComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
}

void UCPP_WeaponAnimComponent::TrySetController(){
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn)
	{
	    Controller = Cast<APlayerController>(OwnerPawn->GetController()); // 获取玩家控制器
	}
}

void UCPP_WeaponAnimComponent::Init(USceneComponent *WeaponRootToSet, USceneComponent *SightToSet, USceneComponent *CameraRootToSet)
{
	WeaponRoot = WeaponRootToSet;
	Sight = SightToSet;
	CameraRoot = CameraRootToSet;
	CurrentBobResult = FVector::ZeroVector;
	if (WeaponRoot && CameraRoot && Sight) {
		//遍历InitializeBases中的每一项并设置为WeaponRoot的相对变换
		for (FName& InitializeBase : InitializeBases) {
		    if (BaseStates.Contains(InitializeBase)){
				BaseStates[InitializeBase] = WeaponRoot->GetRelativeTransform();
			}
		}
		// 设置ADS基准位置
		SetSight(SightToSet, TargetADSXOffset, ADSBaseRotation);
		// 设置摄像机初始位置, 用于计算侧头偏移
		CamInitialLocation = CameraRoot->GetRelativeLocation();
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("WeaponAnimComponent Init failed"));
	}
	TrySetController();
	if (!Controller)
	{
		UE_LOG(LogTemp, Error, TEXT("Weapon Anim Component: Player Controller init failed!"));
	}
	//Base模块初始化
	if (BaseStates.Contains(DefaultBase)){
		TargetBaseTransform = BaseStates[DefaultBase];
		CurrentBaseLocation = TargetBaseTransform.GetLocation();
		CurrentBaseRotation = TargetBaseTransform.GetRotation().Rotator();
	}else{
		SetComponentTickEnabled(false);
		UE_LOG(LogTemp, Error, TEXT("Base default not found, check settings under Base"));
	}
	//Bob模块初始化
	if (BobStates.Contains(DefaultBobStatic) && BobStates.Contains(DefaultBobMovement)){
		CurrentStaticBob = BobStates[DefaultBobStatic];
		CurrentMovementBob = BobStates[DefaultBobMovement];
	}else{
		SetComponentTickEnabled(false);
		UE_LOG(LogTemp, Error, TEXT("Bob default not found, check settings under Bob"));
	}
	//Sway模块初始化
	if (SwayStates.Contains(DefaultSway)){
		CurrentSwayStruct = SwayStates[DefaultSway];
	}else{
		SetComponentTickEnabled(false);
		UE_LOG(LogTemp, Error, TEXT("Sway default not found, check settings under Sway"));
	}
	//Recoil模块初始化
	if (RecoilStates.Contains(DefaultRecoil)){
		CurrentRecoilStruct = RecoilStates[DefaultRecoil];
	}
	UE_LOG(LogTemp, Warning, TEXT("WeaponAnimComponent Init Success"));
}
void UCPP_WeaponAnimComponent::SetSight(USceneComponent* SightToSet, float Offset, FRotator SightRotation){
	Sight = SightToSet;
	TargetADSXOffset = Offset;
	if (WeaponRoot && CameraRoot && Sight) {
		// 设置ADS基准位置
		TargetSightOffset = UKismetMathLibrary::MakeRelativeTransform(Sight->GetComponentTransform(), WeaponRoot->GetComponentTransform()).GetLocation();
		ADSBaseRotation = SightRotation;
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

// Called every frame
void UCPP_WeaponAnimComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//UpdateSettings();
	ElapsedTime += DeltaTime;
	// 更新基准位置和旋转
	CurrentBaseLocation = FMath::Lerp(CurrentBaseLocation, TargetBaseTransform.GetLocation(), SqrtAlpha(DeltaTime, BaseLocationInterpolationRate));
	CurrentBaseRotation = FMath::Lerp(CurrentBaseRotation, TargetBaseTransform.GetRotation().Rotator(), SqrtAlpha(DeltaTime, BaseRotationInterpolationRate));
	// 侧头处理
	UpdateTilt(DeltaTime);
	//UE_LOG(LogTemp, Warning, TEXT("CurrentTiltOffset: %s, CurrentTiltRoll: %f"), *CurrentTiltOffset.ToString(), CurrentTiltRoll);
	// 武器后坐处理
	UpdateRecoil(DeltaTime);
	// 武器摇晃处理
	UpdateBob();
	CurrentBobResult = FMath::Lerp(CurrentBobResult, BobResult, SqrtAlpha(DeltaTime, BobInterpolationRate));
	CurrentBobResultRot = FMath::Lerp(CurrentBobResultRot, BobResultRot, SqrtAlpha(DeltaTime, BobRotationInterpolationRate));
	//CurrentBobResult = FMath::VInterpTo(CurrentBobResult, BobResult, DeltaTime, BobInterpolationRate);
	//CurrentBobResultRot = FMath::RInterpTo(CurrentBobResultRot, BobResultRot, DeltaTime, BobRotationInterpolationRate);
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
	FVector PreADSOffset = CurrentBobResult + CurrentMovementOffset;
	FVector TotalOffset = CurrentRecoilOffset + CurrentBobResult + CurrentMovementOffset + FVector(0.f, 0.f, CurrentJumpOffset) + CurrentRecoilGradualOffset;
	FRotator TotalRotationOffset = RecoilRotationResult + CurrentSway + CurrentBobResultRot + CurrentRecoilGradualRotOffset + FRotator(0.f, 0.f, CurrentMovementRotationOffset);
	// ADS处理
	ADSCorrection(TotalOffset, TotalRotationOffset, DeltaTime);
	if (CurrentADSTime == 0.f){
		//播放关镜动画时用插值
		CurrentADSCorrection = FMath::Lerp(CurrentADSCorrection, TargetADSCorrection, SqrtAlpha(DeltaTime, ADSInterpolationRate));
	}else{
		//播放开镜动画时忽略插值直接设置
		CurrentADSCorrection = TargetADSCorrection;
	}
	//CurrentADSCorrection = FMath::VInterpTo(CurrentADSCorrection, TargetADSCorrection, DeltaTime, ADSInterpolationRate);
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
	RecoilTargetOffset = JitterVector(CurrentRecoilStruct.RecoilOffset, CurrentRecoilStruct.RecoilOffsetJitter);
	RecoilRotationTargetOffset = JitterVector(CurrentRecoilStruct.RecoilRotationOffset, CurrentRecoilStruct.RecoilRotationOffsetJitter);
}

void UCPP_WeaponAnimComponent::StartRecoilAnim()
{
	UpdateRecoilEnd();
	CurrentRecoilTime = 0.f;
	IsPlayingRecoilAnim = true;
	if (Controller){
		if (CurrentRecoilStruct.RecoilCameraShakeClass){
			Controller->PlayerCameraManager->StartCameraShake(CurrentRecoilStruct.RecoilCameraShakeClass, 1.f);
		}
		if (RecoilForceFeedbackEffect){
			Controller->ClientPlayForceFeedback(RecoilForceFeedbackEffect);
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
	TargetBaseTransform.SetRotation(ADSBaseRotation.Quaternion());
}

void UCPP_WeaponAnimComponent::EndADS()
{
	ToADS = false;
	IsAiming = false;
	PlayingADSAnimation = true;
	CurrentADSTime = 0.f;
}
void UCPP_WeaponAnimComponent::UpdateSway()
{
	float Yaw = FMath::Clamp(InputRotator.Yaw * CurrentSwayStruct.SwayYawMultiplier * -1, -CurrentSwayStruct.SwayYawMax / 2, CurrentSwayStruct.SwayYawMax / 2);
	float Pitch = FMath::Clamp(InputRotator.Pitch * CurrentSwayStruct.SwayPitchMultiplier, -CurrentSwayStruct.SwayPitchMax / 2, CurrentSwayStruct.SwayPitchMax / 2);
	TargetSway = FRotator(Pitch, Yaw, 0.f);
}
void UCPP_WeaponAnimComponent::UpdateBob()
{
	// 根据移动状态设置参数
	float multiplier = 1.f;
	MoveSize = InputVector2D.Size();
	multiplier *= BobMultiplier;
	if (MoveSize > 0.01f)
	{
		multiplier = MoveSize;
		CurrentBob = CurrentMovementBob;
	}else{
		CurrentBob = CurrentStaticBob;
	}
	// 计算目标 Bob 位移
	float HorizontalMultiplier = FMath::Sin(ElapsedTime * CurrentBob.BobFrequencyMultiplier * 2 + PI * 0.25) * multiplier;
	float VerticalMultiplier = FMath::Sin(ElapsedTime * CurrentBob.BobFrequencyMultiplier) * multiplier;
	float Noise = FMath::PerlinNoise1D(ElapsedTime * CurrentBob.BobNoiseFrequencyMultiplier) * CurrentBob.BobNoiseLongitude * multiplier;
	if (VerticalMultiplier <= 0.f) VerticalMultiplier *= 0.25f;
	float Z = HorizontalMultiplier * CurrentBob.BobLongitudeZ + Noise;
	float Y = VerticalMultiplier * CurrentBob.BobLongitudeY + Noise;
	float Yaw = FMath::Sin(ElapsedTime * CurrentBob.BobFrequencyMultiplier + PI * 0.25) * CurrentBob.BobYaw * multiplier + Noise;
	float Pitch = FMath::Abs(FMath::Sin(ElapsedTime * CurrentBob.BobFrequencyMultiplier)) * CurrentBob.BobPitch * multiplier + Noise;
	BobResult = FVector(0.f, Y, Z);
	BobResultRot = FRotator(Pitch, Yaw, 0.f);
}
void UCPP_WeaponAnimComponent::ADSCorrection(FVector TotalOffset, FRotator TotalRotationOffset, float DeltaTime)
{
	// 根据曲线和时间计算ADS动画的插值
	CurrentSightOffset = FMath::Lerp(CurrentSightOffset, TargetSightOffset, SqrtAlpha(DeltaTime, SightOffsetInterpolationRate));
	CurrentADSXOffset = FMath::Lerp(CurrentADSXOffset, TargetADSXOffset, SqrtAlpha(DeltaTime, SightOffsetInterpolationRate));
	if (PlayingADSAnimation && ADSCurve) {
		if (ToADS) {
			CurrentADSTime = FMath::Clamp(CurrentADSTime + DeltaTime, 0.f, ADSTime);
			if (CurrentADSTime == ADSTime)
			{
				PlayingADSAnimation = false;
				IsAiming = true;
				UpdateSettings();
			}
		}
		else {
			CurrentADSTime = FMath::Clamp(CurrentADSTime - DeltaTime, 0.f, ADSTime);
			if (CurrentADSTime == 0.f)
			{
				PlayingADSAnimation = false;
				UpdateSettings();
			}
		}
		ADSAlpha = ADSCurve->GetFloatValue(CurrentADSTime / ADSTime);
	}
	// 根据TotalOffset和TotalRotationOffset预测结算后的准星的相对位置
	FVector PredictedSightLocation = CurrentBaseLocation + TotalOffset + CurrentBaseRotation.RotateVector(TotalRotationOffset.RotateVector(CurrentSightOffset));
	FVector PredictedDeviation = PredictedSightLocation - FVector(CurrentADSXOffset, 0.f, 0.f) - CurrentRecoilOffset - CurrentRecoilGradualOffset - FVector(0.f, 0.f, CurrentJumpOffset);
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
void UCPP_WeaponAnimComponent::UpdateSettings_Implementation(){}
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
	CurrentJumpOffset = FMath::Lerp(CurrentJumpOffset, *TargetJumpOffset * 	JumpMultiplier, SqrtAlpha(DeltaTime, *CurrentJumpInterpolationRate));
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
				if (FMath::Abs(CurrentJumpOffset - *TargetJumpOffset * JumpMultiplier) < JumpTransitionTolerance) {
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
				if (FMath::Abs(CurrentJumpOffset - *TargetJumpOffset * JumpMultiplier) < JumpTransitionTolerance) {
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
	float Alpha = SqrtAlpha(DeltaTime, TiltInterpolationRate);
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

float UCPP_WeaponAnimComponent::SqrtAlpha(float DeltaTime, float Rate)
{
	// 使用平方根插值
	float Alpha = 0.f;
	Alpha = FMath::Clamp(DeltaTime * Rate, 0.f, 1.f);
	Alpha = 1 - FMath::Square(1 - Alpha); // 二次缓出
	return Alpha;
}
void UCPP_WeaponAnimComponent::UpdateRecoil(float DeltaTime){
	if (IsPlayingRecoilAnim && RecoilCurve)
	{
		CurrentRecoilTime = FMath::Clamp(CurrentRecoilTime + DeltaTime, 0.f, RecoilAnimTime);
		float alpha = RecoilCurve->GetFloatValue(CurrentRecoilTime / RecoilAnimTime);
		CurrentRecoilOffset = FMath::Lerp(FVector(0.f, 0.f, 0.f), RecoilTargetOffset, alpha);
		FVector RotationVector = FMath::Lerp(FVector(0.f, 0.f, 0.f), RecoilRotationTargetOffset, alpha);
		RecoilRotationResult = FRotator(RotationVector.X, RotationVector.Y, RotationVector.Z);
		CurrentRecoilGradualOffset = FMath::Lerp(CurrentRecoilGradualOffset, CurrentRecoilStruct.RecoilGradualOffset, SqrtAlpha(DeltaTime, CurrentRecoilStruct.RecoilGradualOffsetInterpolationRate));
		CurrentRecoilGradualRotOffset = FMath::Lerp(CurrentRecoilGradualRotOffset, CurrentRecoilStruct.RecoilGradualRotationOffset, SqrtAlpha(DeltaTime, CurrentRecoilStruct.RecoilGradualOffsetInterpolationRate));
		if (CurrentRecoilTime == RecoilAnimTime)
		{
			IsPlayingRecoilAnim = false;
			CurrentRecoilTime = 0.f;
		}
	}else{
		if (!RecoilCurve){
			UE_LOG(LogTemp, Error, TEXT("RecoilCurve nullptr"));
		}
		CurrentRecoilGradualOffset = FMath::Lerp(CurrentRecoilGradualOffset, FVector::ZeroVector, SqrtAlpha(DeltaTime, CurrentRecoilStruct.RecoilGradualOffsetRecoverRate));
		CurrentRecoilGradualRotOffset = FMath::Lerp(CurrentRecoilGradualRotOffset, FRotator::ZeroRotator, SqrtAlpha(DeltaTime, CurrentRecoilStruct.RecoilGradualOffsetRecoverRate));
	}
}

void UCPP_WeaponAnimComponent::SetBase(FName BaseName){
	if (BaseStates.Contains(BaseName)){
		TargetBaseTransform = BaseStates[BaseName];
	}else{
		UE_LOG(LogTemp, Error, TEXT("BaseName %s not found"), *BaseName.ToString());
	}
	if (IsAiming || PlayingADSAnimation){
		TargetBaseTransform.SetRotation(ADSBaseRotation.Quaternion());
	}
}

void UCPP_WeaponAnimComponent::SetRecoil(FName RecoilName){
	if (RecoilStates.Contains(RecoilName)){
		CurrentRecoilStruct = RecoilStates[RecoilName];
	}else{
		UE_LOG(LogTemp, Error, TEXT("RecoilName %s not found"), *RecoilName.ToString());
	}
}

void UCPP_WeaponAnimComponent::SetStaticBob(FName BobName){
	if (BobStates.Contains(BobName)){
		CurrentStaticBob = BobStates[BobName];
	}else{
		UE_LOG(LogTemp, Error, TEXT("BobName %s not found"), *BobName.ToString());
	}
}

void UCPP_WeaponAnimComponent::SetMovementBob(FName BobName){
	if (BobStates.Contains(BobName)){
		CurrentMovementBob = BobStates[BobName];
	}else{
		UE_LOG(LogTemp, Error, TEXT("BobName %s not found"), *BobName.ToString());
	}
}

void UCPP_WeaponAnimComponent::SetSway(FName SwayName){
	if (SwayStates.Contains(SwayName)){
		CurrentSwayStruct = SwayStates[SwayName];
	}else{
		UE_LOG(LogTemp, Error, TEXT("SwayName %s not found"), *SwayName.ToString());
	}
}
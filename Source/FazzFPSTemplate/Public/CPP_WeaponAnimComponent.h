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
	//���������������
	UFUNCTION(BlueprintCallable) void SetInputVector(FVector Vector);
	UFUNCTION(BlueprintCallable) void SetInputRotator(FRotator Rotator);
	FVector2D InputVector2D;
	FVector InputVector;
	FRotator InputRotator;
	float MoveSize;
	//����
	FVector Result;
	FRotator RotationResult;
	FVector JitterVector(FVector Input, FVector Jitter);
	//��׼
	enum class EStanceState
	{
		Default,
		Sprint,
		Crouch
	};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base") TMap<FName, FTransform> BaseStates;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base") FName DefaultBase = "IdleBase"; //Ĭ�ϻ�׼��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base") TArray<FName> InitializeBases = {"IdleBase", "ADSBase"}; //����Ϸ��ʼʱ����WeaponRoot��Transform��ֵ�����Base
	EStanceState CurrentStance = EStanceState::Default;
	UFUNCTION(BlueprintCallable) void StartSprint();
	UFUNCTION(BlueprintCallable) void EndSprint();
	UFUNCTION(BlueprintCallable) void StartCrouch();
	UFUNCTION(BlueprintCallable) void EndCrouch();
	//��ǰ��׼����
	FTransform TargetBaseTransform;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base") float BaseLocationInterpolationRate = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base") float BaseRotationInterpolationRate = 5.f;
	FVector CurrentBaseLocation;
	FRotator CurrentBaseRotation;
	//���������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil") TMap<FName, FWeaponRecoilStruct> RecoilStates;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil") float RecoilAnimTime = 0.2f; //����������ʱ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil") UCurveFloat* RecoilCurve;	// ����������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil") UForceFeedbackEffect* RecoilForceFeedbackEffect; //��������
	float CurrentRecoilTime = 0.0f;
	//������λ��ƫ��
	FVector RecoilTargetOffset;
	FVector GradualRecoilOffsetTarget;
	UPROPERTY(BlueprintReadOnly) FVector CurrentRecoilOffset; // ADS״̬�����������Ӱ�쵽׼��ƫ�ơ���һҪ��������ݶ�ȡ׼�ǣ�����ͼ�ܻ�ȡ����ǰ������ƫ��
	FRotator RecoilRotationResult = FRotator::ZeroRotator; //��������תƫ��
	UPROPERTY(BlueprintReadOnly) FVector CurrentRecoilGradualOffset; //����������ƫ��
	FVector RecoilRotationTargetOffset; //��������תƫ��
	FRotator CurrentRecoilGradualRotOffset; //��������ת����ƫ��
	FWeaponRecoilStruct CurrentRecoilStruct;
	void UpdateRecoilEnd();
	void UpdateRecoil(float DeltaTime);
	UFUNCTION(BlueprintCallable)
	void StartRecoilAnim();
	bool IsPlayingRecoilAnim = false;
	//�����ζ����
	FVector CurrentBobResult;
	FVector BobResult;
	FRotator CurrentBobResultRot;
	FRotator BobResultRot;
	void UpdateBob();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobInterpolationRate = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobRotationInterpolationRate = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") 
	TMap<FName, FWeaponBobStruct> BobStates;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob")  FName DefaultBobStatic = "IdleBob";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob")  FName DefaultBobMovement = "WalkBob";
	FWeaponBobStruct CurrentStaticBob;
	FWeaponBobStruct CurrentMovementBob;
	FWeaponBobStruct CurrentBob;
	float CurrentBobMultiplier = 1.f;
	// Sway���
	FRotator CurrentSway = FRotator::ZeroRotator;
	FRotator TargetSway = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sway") float SwayInterpolationRate = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sway") TMap<FName, FWeaponSwayStruct> SwayStates;
	FWeaponSwayStruct CurrentSwayStruct;
	void UpdateSway();
	// MovementOffset���
	FVector CurrentMovementOffset = FVector::ZeroVector;
	FVector TargetMovementOffset = FVector::ZeroVector;
	float CurrentMovementRotationOffset = 0.f;
	float TargetMovementRotationOffset = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovementOffset") float MovementOffsetInterpolationRate = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovementOffset") FVector MovementOffsetMax = FVector(2.f, 2.f, 5.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovementOffset") float MovementRotationOffsetMax = 5.f;
	void UpdateMovementOffset();
	// ��Ծ���
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump") float JumpTransitionTolerance = 0.7f; // ��Ծ����ת���׶�ʱ�������Χ��ֵԽ��ת��Խ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump") float JumpMultiplier = 1.f; 
	float* CurrentJumpInterpolationRate = &JumpOffsetInterpolationRateUp;
	UFUNCTION(BlueprintCallable) void StartJump();
	UFUNCTION(BlueprintCallable) void MidAir();
	UFUNCTION(BlueprintCallable) void EndJump();
	void UpdateJumpState();
	void UpdateJump(float DeltaTime);
	// ��ͷ���
	FVector CurrentTiltOffset = FVector::ZeroVector;
	UPROPERTY(BlueprintReadonly) float CurrentTiltRoll = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tilt") float TiltInterpolationRate = 5.f; //��ͷ��ֵ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tilt") FVector TiltOffsetLeft = FVector(0.f, -20.f, 0.f); //���ͷƫ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tilt") FVector TiltOffsetRight = FVector(0.f, 20.f, 0.f); //�Ҳ�ͷƫ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tilt") float TiltRoll = 15.f;
	UPROPERTY(BlueprintReadonly) int TiltDirection = 0; // -1Ϊ���ͷ��1Ϊ�Ҳ�ͷ��0Ϊ�޲�ͷ
	UFUNCTION(BlueprintCallable) void SetTilt(int Direction);
	void UpdateTilt(float DeltaTime);
	// ADS���
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
	// ׼��λ������
	void ADSCorrection(FVector TotalOffset, FRotator TotalRotationOffset, float DeltaTime);
	FVector TargetSightOffset;
	FVector CurrentSightOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS") float SightOffsetInterpolationRate = 5.f;
	float ADSAlpha = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS") float ADSInterpolationRate = 5.f; //ADS��ֵ����
};
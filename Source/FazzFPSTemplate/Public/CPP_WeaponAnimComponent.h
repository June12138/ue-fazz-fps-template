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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FAZZFPSTEMPLATE_API UCPP_WeaponAnimComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCPP_WeaponAnimComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	float ElapsedTime = 0.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) bool ShouldPlayAnimation = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Roots") USceneComponent* WeaponRoot = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Roots") USceneComponent* Sight = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Roots") UCameraComponent* CameraRoot = nullptr;
	UFUNCTION(BlueprintCallable) void Init(USceneComponent* WeaponRootToSet, USceneComponent* SightToSet, UCameraComponent* CameraRootToSet);
	void UpdateSettings();
	//���������������
	UFUNCTION(BlueprintCallable) void SetInputVector(FVector Vector);
	UFUNCTION(BlueprintCallable) void SetInputRotator(FRotator Rotator);
	FVector InputVector;
	FRotator InputRotator;
	//��׼
	enum class EStanceState
	{
		Default,
		Sprint,
		Crouch
	};
	EStanceState CurrentStance = EStanceState::Default;
	FVector DefaultBaseLocation;
	FRotator DefaultBaseRotation;
	FVector ADSBaseLocation;
		// ���ܻ�׼����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base") FVector SprintBaseLocation = FVector(25, 5, -15);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base") FRotator SprintBaseRotation = FRotator(-20, -55, 0.f);
	UFUNCTION(BlueprintCallable) void StartSprint();
	UFUNCTION(BlueprintCallable) void EndSprint();
		// �¶׻�׼����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base") FVector CrouchBaseLocation = FVector(25, 3, -10);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base") FRotator CrouchBaseRotation = FRotator(-10, 0.f, 0.f);
	UFUNCTION(BlueprintCallable) void StartCrouch();
	UFUNCTION(BlueprintCallable) void EndCrouch();
		//��ǰ��׼����
	FVector* TargetBaseLocation = &DefaultBaseLocation;
	FRotator* TargetBaseRotation = &DefaultBaseRotation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base") float BaseLocationInterpolationRate = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base") float BaseRotationInterpolationRate = 10;
	FVector CurrentBaseLocation;
	FRotator CurrentBaseRotation;
	//����
	FVector Result;
	FRotator RotationResult;
	FVector JitterVector(FVector Input, FVector Jitter);
	//���������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil") float RecoilAnimTime = 0.2f; //����������ʱ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil") UCurveFloat* RecoilCurve;	// ����������
	float CurrentRecoilTime = 0.0f;
		//������λ��ƫ��
	FVector RecoilTargetOffset;
	UPROPERTY(BlueprintReadOnly) FVector CurrentRecoilOffset; // ADS״̬�����������Ӱ�쵽׼��ƫ�ơ���һҪ��������ݶ�ȡ׼�ǣ�����ͼ�ܻ�ȡ����ǰ������ƫ��
		//��������תƫ��
	FVector RecoilRotationTargetOffset;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Recoil") 
	FWeaponRecoilStruct DefaultRecoilStruct = FWeaponRecoilStruct{
		FVector(-1.f, 0.f, 0.f), //������ֹλ��ƫ��
		FVector(1.f,1.f,0.5), //�������ƫ��
		FVector(1.f,0.f,0.f), //������ֹ��תƫ��
		FVector(1.f,2.f,1.f) //���������תƫ��
	}; //�������ṹ��
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Recoil") 
	FWeaponRecoilStruct ADSRecoilStruct = FWeaponRecoilStruct{
		FVector(-0.5, 0.f, 0.3), //������ֹλ��ƫ��
		FVector(0.25,0.05,0.f), //�������ƫ��
		FVector(0.3,0.0,0.0), //������ֹ��תƫ��
		FVector(0.2,0.2,10.f) //���������תƫ��
	}; //�������ṹ��
	FWeaponRecoilStruct* CurrentRecoilStruct = &DefaultRecoilStruct;
	void UpdateRecoilEnd();
	UFUNCTION(BlueprintCallable)
	void StartRecoilAnim();
	bool IsPlayingRecoilAnim = false;
	//�����ζ����
	FVector CurrentBobResult;
	FVector BobResult;
	FRotator CurrentBobResultRot;
	FRotator BobResultRot;
	void UpdateBob();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobInterpolationRate = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobRotationInterpolationRate = 5;
		//Idle�ζ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") FWeaponBobStruct IdleBob = FWeaponBobStruct{0.75, 1.f, 0.f, 3, 0, 0.5};
		//Walk�ζ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") FWeaponBobStruct WalkBob = FWeaponBobStruct{4, 3, 3, 3, 3, 0.7};
		//ADS Idle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") FWeaponBobStruct IdleBobADS = FWeaponBobStruct{1, 0.f, 0.f, 0.05, 0, 0.15};
		//ADS Walk�ζ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") FWeaponBobStruct WalkBobADS = FWeaponBobStruct{4, 0.f, 0.f, 0.1, 0.2, 0.15};
		//Run�ζ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") FWeaponBobStruct RunBob = FWeaponBobStruct{8, 3, 3, 3, 3, 0.7};
		FWeaponBobStruct* CurrentBob = &IdleBob;
	// Sway���
	FRotator CurrentSway = FRotator::ZeroRotator;
	FRotator TargetSway = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sway") float SwayInterpolationRate = 5;
		// Ĭ����̬��sway
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sway")FWeaponSwayStruct DefaultSway = FWeaponSwayStruct{5, 15, 5, 15};
	FWeaponSwayStruct* CurrentSwayStruct = &DefaultSway;
		// ����״̬��sway
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sway")FWeaponSwayStruct ADSSway = FWeaponSwayStruct{3, 0.3, 3, 0.3};
	void UpdateSway();
	// ADS���
	bool ToADS = false;
	bool PlayingADSAnimation = false;
	bool IsAiming = false;
	FVector CurrentADSCorrection;
	FVector TargetADSCorrection = FVector::ZeroVector;
	FVector CurrentADSOffset = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS") FRotator ADSBaseRotation = FRotator(-0.25, 0.f, 0.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS") UCurveFloat* ADSCurve;
	UFUNCTION(BlueprintCallable) void StartADS();
	UFUNCTION(BlueprintCallable) void EndADS(bool UseCurve = true);
	FTransform SightRelativeTransform;
	float CurrentADSTime = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS") float ADSTime = 0.4;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS") float ADSXOffset = 41;
	// ׼��λ������
	void ADSCorrection(FVector TotalOffset, FRotator TotalRotationOffset, float DeltaTime);
	FVector Sight_RootOffset;
	float ADSAlpha = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS") float ADSInterpolationRate = 10; //ADS��ֵ����
};
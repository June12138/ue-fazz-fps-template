// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "WeaponBobStruct.h"
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
	UPROPERTY(BlueprintReadWrite, EditAnywhere) bool ShouldPlayAnimation = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Roots") USceneComponent* WeaponRoot = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Roots") USceneComponent* Sight = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Roots") UCameraComponent* CameraRoot = nullptr;
	UFUNCTION(BlueprintCallable) void Init(USceneComponent* WeaponRootToSet, USceneComponent* SightToSet, UCameraComponent* CameraRootToSet);
	//���������������
	UFUNCTION(BlueprintCallable) void SetInput(FVector Vector, FRotator Rotator);
	FVector InputVector;
	FRotator InputRotator;
	//��׼
	FVector DefaultLocation;
	FRotator DefaultRotation;
	FVector AimLocation;
	FRotator AimRotation;
	//��ǰ��׼����
	FVector* TargetBaseLocation = &DefaultLocation;
	FRotator* TargetBaseRotation = &DefaultRotation;
	float DefaultBaseInterpolationRate = 5.f;
	float CurrentBaseInterpolationRate = DefaultBaseInterpolationRate;
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
	FVector CurrentRecoilOffset;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Recoil") FVector RecoilOffset = FVector(-8.f, 2.f, 0.f); //������ֹλ��ƫ��
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Recoil") FVector RecoilOffsetJitter = FVector(3.f,3.f,3.f); //�������ƫ�� 
		//��������תƫ��
	FVector CurrentRecoilRotationOffset;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Recoil") FVector RecoilRotationOffset = FVector(0.f,0.f,-3.f); //������ֹ��תƫ��
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Recoil") FVector RecoilRotationOffsetJitter = FVector(2.f,2.f,2.f);//������ת���ƫ�� 
	void UpdateRecoilEnd();
	UFUNCTION(BlueprintCallable)
	void StartRecoilAnim();
	bool IsPlayingRecoilAnim = false;
	//�����ζ����
	float BobFrequencyMultiplierCurrent = 0.75;
	float BobLongitudeCurrentZ = 1.f;
	float BobLongitudeCurrentY = 0.f;
	float BobNoiseCurrent = 0.5;
	FVector CurrentBobResult;
	FVector BobResult;
	FRotator CurrentBobResultRot;
	FRotator BobResultRot;
	void UpdateBob();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobInterpolationRate = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") float BobRotationInterpolationRate = 2;
	//Idle�ζ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") FWeaponBobStruct IdleBob = FWeaponBobStruct{0.75, 1.f, 0.f, 3, 0, 0.5};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bob") FWeaponBobStruct WalkBob = FWeaponBobStruct{4, 3, 3, 3, 3, 0.7};
	FWeaponBobStruct* CurrentBob = &IdleBob;
	// Sway���
	FRotator CurrentSway = FRotator::ZeroRotator;
	FRotator TargetSway = FRotator::ZeroRotator;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sway") float SwayInterpolationRate = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sway") float SwayYawMultiplier = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sway") float SwayYawMax = 15;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sway") float SwayPitchMultiplier = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sway") float SwayPitchMax = 15;
	void UpdateSway();
	// ADS���
	bool ToADS = false;
	bool PlayingADSAnimation = false;
	bool IsAiming = false;
	FVector CurrentADSOffset = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS") UCurveFloat* ADSCurve;
	UFUNCTION(BlueprintCallable) void StartADS();
	UFUNCTION(BlueprintCallable) void EndADS();
	FTransform SightRelativeTransform;
	float CurrentADSTime = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS") float ADSTime = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS") float ADSXOffset = 50;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
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
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Roots") USceneComponent* WeaponRoot = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Roots") UCameraComponent* CameraRoot = nullptr;
	FVector StartLocation;
	FVector Result;
	//���������
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Recoil") FVector RecoilOffset; //������ֹλ��
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Recoil") FVector RecoilOffsetJitter; //�������ƫ�� 
	FVector CurrentRecoilOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil") UCurveFloat* RecoilCurve;	// ����������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil") float RecoilAnimTime;
	float CurrentRecoilTime = 0.0f;
	void UpdateRecoilEnd();
	UFUNCTION(BlueprintCallable)
	void StartRecoilAnim();
	bool IsPlayingRecoilAnim = false;
	UFUNCTION(BlueprintCallable) void Init(USceneComponent* WeaponRootToSet, UCameraComponent* CameraRootToSet);
};

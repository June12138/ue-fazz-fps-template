// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponAnimComponent_CPP.h"

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
	if (WeaponRoot) {
		StartLocation = WeaponRoot->GetRelativeLocation();
		StartRotation = FVector(WeaponRoot->GetRelativeRotation().Pitch, WeaponRoot->GetRelativeRotation().Yaw, WeaponRoot->GetRelativeRotation().Roll);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("WeaponRoot is null"));
	}
}

// Called every frame
void UWeaponAnimComponent_CPP::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	FVector RecoilResult = FVector(0.f,0.f,0.f);
	FVector RecoilRotationResult = FVector(0.f, 0.f, 0.f);
	//后坐力处理
	if (IsPlayingRecoilAnim && RecoilCurve) {
		CurrentRecoilTime = FMath::Clamp(CurrentRecoilTime + DeltaTime, 0.f, RecoilAnimTime);
		float alpha = RecoilCurve->GetFloatValue(CurrentRecoilTime / RecoilAnimTime);
		//UE_LOG(LogTemp, Warning, TEXT("Alpha: %f"), alpha);
		RecoilResult = FMath::Lerp(FVector(0.f, 0.f, 0.f), CurrentRecoilOffset, alpha);
		RecoilRotationResult = FMath::Lerp(FVector(0.f, 0.f, 0.f), CurrentRecoilRotationOffset, alpha);
		if (CurrentRecoilTime == RecoilAnimTime) { 
			IsPlayingRecoilAnim = false; 
			CurrentRecoilTime = 0.f;
		}
	}
	Result = StartLocation + RecoilResult;
	WeaponRoot->SetRelativeLocation(Result);
	RotationResult = StartRotation + RecoilRotationResult;
	WeaponRoot->SetRelativeRotation(FRotator(RotationResult.X, RotationResult.Y, RotationResult.Z));
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
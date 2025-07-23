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


// Called every frame
void UWeaponAnimComponent_CPP::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...
	if (IsPlayingRecoilAnim && RecoilCurve) {
		CurrentRecoilTime = FMath::Clamp(CurrentRecoilTime + DeltaTime, 0.f, RecoilAnimTime);
		float alpha = RecoilCurve->GetFloatValue(CurrentRecoilTime / RecoilAnimTime);
		//UE_LOG(LogTemp, Warning, TEXT("Alpha: %f"), alpha);
		WeaponRoot->SetRelativeLocation(FMath::Lerp(BaseStartLocation, CurrentRecoilOffset, alpha));
		if (CurrentRecoilTime == RecoilAnimTime) { 
			IsPlayingRecoilAnim = false; 
			CurrentRecoilTime = 0.f;
		}
	}
}

FVector UWeaponAnimComponent_CPP::GetRecoilEnd()
{
	float X = FMath::RandRange(RecoilOffset.X - RecoilOffsetJitter.X / 2, RecoilOffset.X + RecoilOffsetJitter.X / 2);
	float Y = FMath::RandRange(RecoilOffset.Y - RecoilOffsetJitter.Y / 2, RecoilOffset.Y + RecoilOffsetJitter.Y / 2);
	float Z = FMath::RandRange(RecoilOffset.Z - RecoilOffsetJitter.Z / 2, RecoilOffset.Z + RecoilOffsetJitter.Z / 2);
	CurrentRecoilOffset = BaseStartLocation - FVector(X, Y, Z);
	return CurrentRecoilOffset;
}

void UWeaponAnimComponent_CPP::StartRecoilAnim()
{
	FVector End = GetRecoilEnd();
	IsPlayingRecoilAnim = true;
	///UE_LOG(LogTemp, Log, TEXT(""));
}

void UWeaponAnimComponent_CPP::Init(USceneComponent* WeaponRootToSet, UCameraComponent* CameraRootToSet)
{
	WeaponRoot = WeaponRootToSet;
	CameraRoot = CameraRootToSet;
	if (WeaponRoot) {
		BaseStartLocation = WeaponRoot->GetRelativeLocation();
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("WeaponRoot is null"));
	}
}

